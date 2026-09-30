#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/cocos/touch_dispatcher/CCTouchDelegateProtocol.h>
#include <Geode/cocos/touch_dispatcher/CCTouchDispatcher.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/binding/GameObject.hpp>

using namespace geode::prelude;
using namespace cocos2d;

namespace {
    GameObject* findObjectAt(CCNode* node, CCPoint worldPoint) {
        if (!node || !node->isVisible())
            return nullptr;

        auto children = node->getChildren();
        if (children) {
            for (int i = static_cast<int>(children->count()) - 1; i >= 0; --i) {
                auto child = static_cast<CCNode*>(children->objectAtIndex(i));
                if (auto* hit = findObjectAt(child, worldPoint))
                    return hit;
            }
        }

        auto* object = typeinfo_cast<GameObject*>(node);
        if (!object || !object->getParent())
            return nullptr;

        auto local = object->convertToNodeSpace(worldPoint);
        auto size = object->getContentSize();
        CCRect rect(0.f, 0.f, size.width, size.height);

        if (rect.containsPoint(local))
            return object;

        return nullptr;
    }

    class FingerDragLayer final : public CCLayer {
    protected:
        GameObject* m_dragged = nullptr;
        CCPoint m_grabOffset = CCPointZero;

        bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
            auto* editor = typeinfo_cast<LevelEditorLayer*>(this->getParent());
            if (!editor)
                return false;

            auto* object = findObjectAt(editor, touch->getLocation());
            if (!object || !object->getParent())
                return false;

            m_dragged = object;

            auto parentPoint = object->getParent()->convertToNodeSpace(touch->getLocation());
            m_grabOffset = object->getPosition() - parentPoint;

            return true;
        }

        void ccTouchMoved(CCTouch* touch, CCEvent*) override {
            if (!m_dragged || !m_dragged->getParent())
                return;

            auto parentPoint = m_dragged->getParent()->convertToNodeSpace(touch->getLocation());
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
            CCTouchDispatcher::get()->addTargetedDelegate(this, -100000, true);
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

            this->setTouchEnabled(true);
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

        dragLayer->setContentSize(this->getContentSize());
        dragLayer->setPosition(CCPointZero);
        this->addChild(dragLayer, 1000000);

        log::info("Finger Drag: editor touch layer enabled");
        return true;
    }
};
