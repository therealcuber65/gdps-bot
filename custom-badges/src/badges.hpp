inline CCNode* createBadgeNode(const BadgeDef& badge, float targetHeight = 20.f) {
    if (badge.isVanilla) {
        if (auto spr = CCSprite::createWithSpriteFrameName(badge.vanillaFrame.c_str())) {
            spr->setScale(targetHeight / spr->getContentSize().height);
            return spr;
        }
    }
    if (!badge.file.empty()) {
        auto imgPath = Mod::get()->getResourcesDir() / badge.file;
        std::error_code ec;
        if (std::filesystem::exists(imgPath, ec) && std::filesystem::file_size(imgPath, ec) > 0) {
            auto sprName = fmt::format("{}"_spr, badge.file);
            if (auto spr = CCSprite::create(sprName.c_str())) {
                spr->setScale(targetHeight / spr->getContentSize().height);
                return spr;
            }
        }
    }
    auto lbl = CCLabelBMFont::create(badge.name.c_str(), "bigFont.fnt");
    lbl->setScale(0.24f);
    lbl->setColor(badge.fallbackColor);
    return lbl;
}
