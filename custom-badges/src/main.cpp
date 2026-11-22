#include "popup.hpp"
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <chrono>

void fetchRemoteBadges(std::function<void()> onDone = nullptr) {
    static async::TaskHolder<web::WebResponse> listener;
    web::WebRequest req;
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    std::string url = fmt::format(
        "https://githubusercontent.com{}",
        now
    );
    listener.spawn(
        req.get(url),
        [onDone = std::move(onDone)](web::WebResponse res) {
            if (!res.ok()) return;
            auto rawStr = res.string().unwrapOr("");
            auto parsed = matjson::parse(rawStr);
            if (!parsed) return;

            std::unordered_map<int, std::vector<int>> nextMap;
            for (auto& [key, val] : parsed.unwrap()) {
                if (!val.isArray()) continue;
                try {
                    int accID = std::stoi(key);
                    std::vector<int> ids;
                    for (auto& entry : val) {
                        if (entry.isNumber()) ids.push_back(static_cast<int>(entry.asInt().unwrap()));
                    }
                    if (!ids.empty()) nextMap[accID] = std::move(ids);
                } catch (...) {}
            }
            g_remoteBadges = std::move(nextMap);
            if (onDone) onDone();
        }
    );
}

$on_mod(Loaded) {
    fetchRemoteBadges();
}

class $modify(MyProfilePage, ProfilePage) {
    void refreshCustomBadges() {
        if (!m_score) return;

        auto usernameMenu = m_mainLayer->getChildByIDRecursive("username-menu");
        if (!usernameMenu) return;

        std::vector<CCNode*> toRemove;
        for (auto* child : CCArrayExt<CCNode*>(usernameMenu->getChildren())) {
            if (child->getID().starts_with("custom-profile-badge-"_spr)) {
                toRemove.push_back(child);
            }
        }
        for (auto* child : toRemove) child->removeFromParent();

        auto ids = getAssignedBadgeIDs(m_score->m_accountID);
        if (auto vanillaBadge = usernameMenu->getChildByID("mod-badge")) {
            vanillaBadge->setVisible(ids.empty());
        }

        int order = -100;
        for (int id : ids) {
            if (auto badge = findBadgeByID(id)) {
                auto node = createBadgeNode(*badge, 20.f);
                node->setID(fmt::format("custom-profile-badge-{}"_spr, id));
                node->setZOrder(order++);
                usernameMenu->addChild(node);
            }
        }
        usernameMenu->updateLayout();
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);
        if (auto leftMenu = m_mainLayer->getChildByID("left-menu")) {
            if (!leftMenu->getChildByID("badge-manager-btn"_spr)) {
                auto spr = CircleButtonSprite::createWithSpriteFrameName(
                    "modBadge_01_001.png", 1.f, CircleBaseColor::Green, CircleBaseSize::Small
                );
                auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MyProfilePage::onOpenBadgeMenu));
                btn->setID("badge-manager-btn"_spr);
                leftMenu->addChild(btn);
                leftMenu->updateLayout();
            }
        }
        this->refreshCustomBadges();
        Ref<MyProfilePage> self = this;
        fetchRemoteBadges([self]() {
            if (self) self->refreshCustomBadges();
        });
    }

    void onOpenBadgeMenu(CCObject*) {
        if (!m_score) return;
        BadgeSelectPopup::create(m_score->m_accountID, m_score->m_userName, [this]() {
            this->refreshCustomBadges();
        })->show();
    }
};

class $modify(MyCommentCell, CommentCell) {
    void loadFromComment(GJComment* comment) {
        CommentCell::loadFromComment(comment);
        if (!comment) return;
        auto ids = getAssignedBadgeIDs(comment->m_accountID);
        if (ids.empty()) return;

        if (auto usernameMenu = m_mainLayer->getChildByIDRecursive("username-menu")) {
            if (auto vanillaBadge = usernameMenu->getChildByID("mod-badge")) {
                vanillaBadge->setVisible(false);
            }
            for (int id : ids) {
                if (auto badge = findBadgeByID(id)) {
                    auto node = createBadgeNode(*badge, 13.f);
                    node->setID(fmt::format("custom-comment-badge-{}"_spr, id));
                    usernameMenu->addChild(node);
                }
            }
            usernameMenu->updateLayout();
        }
    }
};
