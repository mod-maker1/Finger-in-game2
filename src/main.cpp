#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/cocos/touch_dispatcher/CCTouchDispatcher.h>
#include <Geode/binding/GameObject.hpp>

using namespace geode::prelude;
using namespace cocos2d;

namespace {
    GameObject* findObjectAt(CCNode* node, CCPoint pointInNode) {
        if (!node || !node->isVisible())
            return nullptr;

        auto children = node->getChildren();
        if (!children)
            return nullptr;

        auto worldPoint = node->convertToWorldSpace(pointInNode);

        for (int i = static_cast<int>(children->count()) - 1; i >= 0; --i) {
            auto* child = static_cast<CCNode*>(children->objectAtIndex(i));

            if (!child || !child->isVisible())
                continue;

            auto pointInChild = child->convertToNodeSpace(worldPoint);

            if (auto* object = typeinfo_cast<GameObject*>(child)) {
                auto size = object->getContentSize();

                if (size.width > 0.f && size.height > 0.f) {
                    CCRect rect(0.f, 0.f, size.width, size.height);

                    if (rect.containsPoint(pointInChild))
                        return object;
                }
            }

            if (auto* hit = findObjectAt(child, pointInChild))
                return hit;
        }

        return nullptr;
    }

    class FingerDragLayer final : public CCLayer {
        GameObject* m_dragged = nullptr;
        CCPoint m_grabOffset = CCPointZero;

        bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
            auto* editor = typeinfo_cast<LevelEditorLayer*>(getParent());

            if (!editor)
                return false;

            auto editorPoint =
                editor->convertToNodeSpace(touch->getLocation());

            m_dragged = findObjectAt(editor, editorPoint);

            if (!m_dragged || !m_dragged->getParent()) {
                m_dragged = nullptr;
                return false;
            }

            auto parentPoint =
                m_dragged->getParent()->convertToNodeSpace(
                    touch->getLocation()
                );

            m_grabOffset = m_dragged->getPosition() - parentPoint;

            log::info("Finger Drag: object grabbed");

            return true;
        }

        void ccTouchMoved(CCTouch* touch, CCEvent*) override {
            if (!m_dragged || !m_dragged->getParent())
                return;

            auto parentPoint =
                m_dragged->getParent()->convertToNodeSpace(
                    touch->getLocation()
                );

            m_dragged->setPosition(parentPoint + m_grabOffset);
        }

        void ccTouchEnded(CCTouch*, CCEvent*) override {
            m_dragged = nullptr;
            m_grabOffset = CCPointZero;
        }

        void ccTouchCancelled(CCTouch*, CCEvent*) override {
            m_dragged = nullptr;
            m_grabOffset = CCPointZero;
        }

        void registerWithTouchDispatcher() override {
            CCTouchDispatcher::get()->addTargetedDelegate(
                this,
                -1000000,
                true
            );
        }

    public:
        static FingerDragLayer* create() {
            auto* ret = new FingerDragLayer();

            if (ret && ret->init()) {
                ret->autorelease();
                return ret;
            }

            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        bool init() {
            if (!CCLayer::init())
                return false;

            setTouchEnabled(true);
            return true;
        }
    };
}

class $modify(FingerDragLevelEditorLayer, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI))
            return false;

        auto* dragLayer = FingerDragLayer::create();

        if (!dragLayer)
            return true;

        dragLayer->setContentSize(getContentSize());
        dragLayer->setPosition(CCPointZero);

        addChild(dragLayer, 1000000);

        log::info("Finger Drag: editor touch layer enabled");

        return true;
    }
};
