#include "popup.hpp"
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>

void fetchRemoteBadges(std::function<void()> onDone = nullptr) {
    static async::TaskHolder<web::WebResponse> listener;
    web::WebRequest req;
    std::string url = std::string("https://") + "://githubusercontent.com";
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
        if (auto oldRow = m_mainLayer->getChildByID("custom-profile-badges-row"_spr)) {
            oldRow->removeFromParent();
        }
        auto ids = getAssignedBadgeIDs(m_score->m_accountID);
        if (auto vanillaBadge = m_mainLayer->getChildByID("mod-badge")) {
            vanillaBadge->setVisible(ids.empty());
        }
        if (ids.empty()) return;

        if (auto usernameLabel = m_mainLayer->getChildByID("username-label")) {
            auto row = CCNode::create();
            row->setID("custom-profile-badges-row"_spr);

            float cursorX = 0.f;
            float spacing = 4.f;
            for (int id : ids) {
                if (auto badge = findBadgeByID(id)) {
                    auto node = createBadgeNode(*badge, 18.f);
                    float w = node->getContentSize().width * node->getScaleX();
                    node->setPosition({ cursorX - (w / 2.f), 0.f });
                    row->addChild(node);
                    cursorX -= (w + spacing);
                }
            }

            float halfWidth = (usernameLabel->getContentSize().width * usernameLabel->getScaleX()) / 2.f;
            row->setPosition({
                usernameLabel->getPositionX() - halfWidth - 10.f,
                usernameLabel->getPositionY()
            });
            m_mainLayer->addChild(row, 10);
        }
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
