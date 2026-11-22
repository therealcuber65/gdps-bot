#pragma once
#include "badges.hpp"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <cmath>

class BadgeSelectPopup : public geode::Popup {
protected:
    int m_targetAccountID = 0;
    std::string m_targetUsername;
    std::function<void()> m_onChanged;
    geode::ScrollLayer* m_scroll = nullptr;

    void rebuildGrid() {
        if (!m_scroll) return;
        float prevY = m_scroll->m_contentLayer->getPositionY();
        m_scroll->m_contentLayer->removeAllChildrenWithCleanup(true);

        const int cols = 5;
        const float scrollW = 350.f;
        const float scrollH = 195.f;
        const float cellW = scrollW / cols;
        const float cellH = 82.f;

        int rows = static_cast<int>(std::ceil(static_cast<float>(ALL_BADGES.size()) / cols));
        float contentH = std::max(scrollH, rows * cellH + 10.f);
        m_scroll->m_contentLayer->setContentSize({ scrollW, contentH });

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize({ scrollW, contentH });
        m_scroll->m_contentLayer->addChild(menu);

        for (size_t i = 0; i < ALL_BADGES.size(); ++i) {
            const auto& badge = ALL_BADGES[i];
            int r = static_cast<int>(i) / cols;
            int c = static_cast<int>(i) % cols;

            float cx = (c + 0.5f) * cellW;
            float cy = contentH - 5.f - (r + 0.5f) * cellH;
            bool equipped = hasBadgeID(m_targetAccountID, badge.id);

            auto cardBg = CCScale9Sprite::create("square02b_001.png");
            cardBg->setContentSize({ cellW - 6.f, cellH - 6.f });
            cardBg->setColor({ 0, 0, 0 });
            cardBg->setOpacity(75);
            cardBg->setPosition({ cx, cy });
            m_scroll->m_contentLayer->addChild(cardBg, 0);

            auto icon = createBadgeNode(badge, 30.f);
            icon->setPosition({ cx, cy + 17.f });
            m_scroll->m_contentLayer->addChild(icon, 2);

            auto nameLbl = CCLabelBMFont::create(badge.name.c_str(), "goldFont.fnt");
            nameLbl->limitLabelWidth(cellW - 10.f, 0.26f, 0.1f);
            nameLbl->setPosition({ cx, cy - 3.f });
            m_scroll->m_contentLayer->addChild(nameLbl, 2);

            auto chkSpr = CCSprite::createWithSpriteFrameName(
                equipped ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png"
            );
            chkSpr->setScale(0.65f);

            auto chkBtn = CCMenuItemSpriteExtra::create(chkSpr, this, menu_selector(BadgeSelectPopup::onToggleBadge));
            chkBtn->setTag(badge.id);
            chkBtn->setPosition({ cx, cy - 22.f });
            menu->addChild(chkBtn);
        }

        m_scroll->m_contentLayer->setPositionY(prevY);
    }

    bool init(int accountID, std::string username, std::function<void()> onChanged) {
        if (!Popup::init(390.f, 260.f)) return false;
        m_targetAccountID = accountID;
        m_targetUsername = std::move(username);
        m_onChanged = std::move(onChanged);
        this->setTitle(fmt::format("Badges: {}", m_targetUsername));

        auto clearSpr = ButtonSprite::create("Clear", "goldFont.fnt", "GJ_button_06.png", 0.65f);
        clearSpr->setScale(0.55f);
        auto clearBtn = CCMenuItemSpriteExtra::create(clearSpr, this, menu_selector(BadgeSelectPopup::onClearAll));
        clearBtn->setPosition({ 348.f, 238.f });
        m_buttonMenu->addChild(clearBtn);

        auto bg = CCScale9Sprite::create("square02b_001.png");
        bg->setContentSize({ 356.f, 201.f });
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(95);
        bg->setPosition({ 195.f, 118.f });
        m_mainLayer->addChild(bg);

        m_scroll = geode::ScrollLayer::create({ 350.f, 195.f });
        m_scroll->setPosition({ 20.f, 20.f });
        m_mainLayer->addChild(m_scroll);

        this->rebuildGrid();
        m_scroll->moveToTop();
        return true;
    }

    void onToggleBadge(CCObject* sender) {
        toggleBadgeID(m_targetAccountID, sender->getTag());
        this->rebuildGrid();
        if (m_onChanged) m_onChanged();
    }

    void onClearAll(CCObject*) {
        saveAssignedBadgeIDs(m_targetAccountID, {});
        this->rebuildGrid();
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
