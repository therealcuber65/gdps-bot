void fetchRemoteBadges(std::function<void()> onDone = nullptr) {
    static async::TaskHolder<web::WebResponse> listener;
    web::WebRequest req;
    listener.spawn(
        req.get("https://githubusercontent.com"),
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
