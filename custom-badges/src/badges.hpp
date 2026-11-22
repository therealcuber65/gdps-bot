#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <algorithm>
#include <sstream>
#include <unordered_map>

using namespace geode::prelude;

struct BadgeDef {
    int id;
    std::string name;
    std::string file;
    bool isVanilla;
    std::string vanillaFrame;
    ccColor3B fallbackColor;
};

static const std::vector<BadgeDef> ALL_BADGES = {
    { 1,  "Moderator (Vanilla)",          "", true,  "modBadge_01_001.png", { 255, 215, 0   } },
    { 2,  "Elder Moderator (Vanilla)",    "", true,  "modBadge_02_001.png", { 255, 110, 60  } },
    { 3,  "Leaderboard Mod (Vanilla)",    "", true,  "modBadge_03_001.png", { 80,  180, 255 } },
    { 4,  "Owner",                        "owner.png",            false, "", { 255, 70,  70  } },
    { 5,  "Rater",                        "rater.png",            false, "", { 255, 200, 60  } },
    { 6,  "Leaderboard Mod: Owner",       "lb_owner.png",         false, "", { 80,  220, 255 } },
    { 7,  "Leaderboard Mod: Co Owner",    "ld_co_owner.png",      false, "", { 80,  190, 255 } },
    { 8,  "Leaderboard Mod: Head Admin",  "lb_head_admin.png",    false, "", { 110, 160, 255 } },
    { 9,  "Leaderboard Mod: Admin",       "lb_admin.png",         false, "", { 140, 140, 255 } },
    { 10, "Partner",                      "partner.png",          false, "", { 120, 255, 140 } },
    { 11, "Bot",                          "bot.png",              false, "", { 180, 180, 195 } },
    { 12, "Booster",                      "booster.png",          false, "", { 245, 120, 255 } },
    { 13, "Daily Winner",                 "daily_winner.png",     false, "", { 255, 220, 90  } },
    { 14, "Weekly Winner",                "weekly_winner.png",    false, "", { 255, 150, 70  } },
    { 15, "Best Player",                  "best_player.png",      false, "", { 90,  255, 220 } },
    { 16, "Best Creator",                 "best_creator.png",     false, "", { 255, 180, 90  } },
    { 17, "Event Winner",                 "event_winner.png",     false, "", { 255, 100, 180 } },
    { 18, "Verified Creator",             "verified_creator.png", false, "", { 100, 230, 130 } },
    { 19, "Verified Player",              "verified_player.png",  false, "", { 100, 200, 255 } },
    { 20, "Lv:1 Promoter",                "promoter_1.png",       false, "", { 210, 210, 210 } },
    { 21, "Lv:2 Promoter",                "promoter_2.png",       false, "", { 190, 235, 190 } },
    { 22, "Lv:3 Promoter",                "promoter_3.png",       false, "", { 160, 230, 255 } },
    { 23, "Lv:4 Promoter",                "promoter_4.png",       false, "", { 170, 190, 255 } },
    { 24, "Lv:5 Promoter",                "promoter_5.png",       false, "", { 210, 170, 255 } },
    { 25, "Lv:6 Promoter",                "promoter_6.png",       false, "", { 255, 170, 230 } },
    { 26, "Lv:7 Promoter",                "promoter_7.png",       false, "", { 255, 210, 140 } },
    { 27, "Lv:8 Promoter",                "promoter_8.png",       false, "", { 255, 160, 120 } },
    { 28, "Lv:9 Promoter",                "promoter_9.png",       false, "", { 255, 110, 110 } },
    { 29, "Lv:10 Promoter",               "promoter_10.png",      false, "", { 255, 225, 70  } }
};

inline std::unordered_map<int, std::vector<int>> g_remoteBadges;

inline std::vector<int> getAssignedBadgeIDs(int accountID) {
    std::vector<int> result;
    if (accountID <= 0) return result;
    if (auto it = g_remoteBadges.find(accountID); it != g_remoteBadges.end()) {
        result = it->second;
    }
    auto raw = Mod::get()->getSavedValue<std::string>(fmt::format("badges_{}", accountID), "");
    std::stringstream ss(raw);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            try {
                int id = std::stoi(item);
                if (std::find(result.begin(), result.end(), id) == result.end()) result.push_back(id);
            } catch (...) {}
        }
    }
    return result;
}

inline void saveAssignedBadgeIDs(int accountID, const std::vector<int>& ids) {
    if (accountID <= 0) return;
    std::string serialized;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i > 0) serialized += ",";
        serialized += std::to_string(ids[i]);
    }
    Mod::get()->setSavedValue<std::string>(fmt::format("badges_{}", accountID), serialized);
}

inline bool hasBadgeID(int accountID, int badgeID) {
    auto ids = getAssignedBadgeIDs(accountID);
    return std::find(ids.begin(), ids.end(), badgeID) != ids.end();
}

inline void toggleBadgeID(int accountID, int badgeID) {
    auto ids = getAssignedBadgeIDs(accountID);
    auto it = std::find(ids.begin(), ids.end(), badgeID);
    if (it != ids.end()) ids.erase(it);
    else ids.push_back(badgeID);
    saveAssignedBadgeIDs(accountID, ids);
}

inline const BadgeDef* findBadgeByID(int badgeID) {
    for (const auto& b : ALL_BADGES) if (b.id == badgeID) return &b;
    return nullptr;
}

inline CCNode* createBadgeNode(const BadgeDef& badge, float targetHeight = 20.f) {
    if (badge.isVanilla) {
        CCSpriteFrameCache::get()->addSpriteFramesWithFile("GJ_GameSheet03.plist");
        if (auto spr = CCSprite::createWithSpriteFrameName(badge.vanillaFrame.c_str())) {
            spr->setScale(targetHeight / spr->getContentSize().height);
            return spr;
        }
    }
    if (!badge.file.empty()) {
        auto imgPath = Mod::get()->getResourcesDir() / badge.file;
        std::error_code ec;
        if (std::filesystem::exists(imgPath, ec) && std::filesystem::file_size(imgPath, ec) > 0) {
            if (auto spr = CCSprite::create(imgPath.string().c_str())) {
                spr->setScale(targetHeight / spr->getContentSize().height);
                return spr;
            }
        }
    }
    auto lbl = CCLabelBMFont::create(badge.name.c_str(), "bigFont.fnt");
    lbl->limitLabelWidth(52.f, 0.22f, 0.08f);
    lbl->setColor(badge.fallbackColor);
    return lbl;
}
