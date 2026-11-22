#pragma once
#include "badges.hpp"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

class BadgeSelectPopup : public geode::Popup {
protected:
    int m_targetAccountID = 0;
    std::string m_targetUsername;
    std::function<void()> m_onChanged;
    ScrollLayer* m_scroll = nullptr;

    void rebuildList() {
        if (!m_scroll) return;
        float prevY = m_scroll->m_contentLayer->getPositionY();
        m_scroll->m_contentLayer->removeAllChildrenWithCleanup(true);

        float rowHeight = 30.f;
        int totalRows = static_cast<int>(ALL_BADGES.size()) + 1;
        float contentHeight = std::max(185.f, totalRows * rowHeight);
        m_scroll->m_contentLayer->setContentSize({ 320.f, contentHeight });

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize({ 320.f, contentHeight });
        m_scroll->m_contentLayer->addChild(menu);

        float clearY = contentHeight - (rowHeight * 0.5f);
        auto clearBg = CCScale9Sprite::create("GJ_square01.png");
        clearBg->setContentSize({ 310.f, 25.f });
        clearBg->setColor({ 150, 50, 50 });
        clearBg->setOpacity(180);

        auto clearBtn = CCMenuItemSpriteExtra::create(clearBg, this, menu_selector(BadgeSelectPopup::onClearAll));
        clearBtn->setPosition({ 160.f, clearY });
        menu->addChild(clearBtn);

        auto clearLbl = CCLabelBMFont::create("Clear All Badges", "bigFont.fnt");
        clearLbl->setScale(0.35f);
        clearLbl->setPosition({ 160.f, clearY });
        m_scroll->m_contentLayer->addChild(clearLbl, 2);

        for (size_t i = 0; i < ALL_BADGES.size(); ++i) {
            const auto& badge = ALL_BADGES[i];
            float y = contentHeight - (rowHeight * (static_cast<float>(i) + 1.5f));
            bool equipped = hasBadgeID(m_targetAccountID, badge.id);

            auto bg = CCScale9Sprite::create("GJ_square01.png");
            bg->setContentSize({ 310.f, 25.f });
            bg->setColor(equipped ? ccColor3B{ 55, 165, 80 } : ccColor3B{ 40, 45, 65 });
            bg->setOpacity(185);

            auto btn = CCMenuItemSpriteExtra::create(bg, this, menu_selector(BadgeSelectPopup::onToggleBadge));
            btn->setTag(badge.id);
            btn->setPosition({ 160.f, y });
            menu->addChild(btn);

            auto icon = createBadgeNode(badge, 18.f);
            icon->setPosition({ 34.f, y });
            m_scroll->m_contentLayer->addChild(icon, 2);

            auto nameLbl = CCLabelBMFont::create(badge.name.c_str(), "bigFont.fnt");
            nameLbl->setAnchorPoint({ 0.f, 0.5f });
            nameLbl->limitLabelWidth(195.f, 0.36f, 0.1f);
            nameLbl->setPosition({ 64.f, y });
            m_scroll->m_contentLayer->addChild(nameLbl, 2);

            if (equipped) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.55f);
                check->setPosition({ 292.f, y });
                m_scroll->m_contentLayer->addChild(check, 2);
            }
        }
        m_scroll->m_contentLayer->setPositionY(prevY);
    }

    bool init(int accountID, std::string username, std::function<void()> onChanged) {
        if (!Popup::init(360.f, 250.f)) return false;
        m_targetAccountID = accountID;
        m_targetUsername = std::move(username);
        m_onChanged = std::move(onChanged);
        this->setTitle(fmt::format("Badges: {} (Multi-Select)", m_targetUsername));

        m_scroll = ScrollLayer::create({ 320.f, 185.f });
        m_scroll->setPosition({ 20.f, 22.f });
        m_mainLayer->addChild(m_scroll);

        this->rebuildList();
        m_scroll->moveToTop();
        return true;
    }

    void onToggleBadge(CCObject* sender) {
        toggleBadgeID(m_targetAccountID, sender->getTag());
        this->rebuildList();
        if (m_onChanged) m_onChanged();
    }

    void onClearAll(CCObject*) {
        saveAssignedBadgeIDs(m_targetAccountID, {});
        this->rebuildList();
        if (m_onChanged) m_onChanged();
    }

public:
    static BadgeSelectPopup* create(int accountID, std::string username, std::function<void()> onChanged) {
        auto ret = new BadgeSelectPopup();
        if (ret && ret->init(accountID, std::move(username), std::move(onChanged))) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
