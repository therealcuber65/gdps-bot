const { Client, GatewayIntentBits, REST, Routes, SlashCommandBuilder, PermissionFlagsBits } = require('discord.js');
const BADGES = require('./badge-list.json');

const CONFIG = {
    DISCORD_TOKEN: "YOUR_DISCORD_BOT_TOKEN",
    CLIENT_ID: "YOUR_DISCORD_APPLICATION_ID",
    GUILD_ID: "YOUR_DISCORD_SERVER_ID",
    GITHUB_TOKEN: "YOUR_GITHUB_PAT_TOKEN",
    GITHUB_OWNER: "therealcuber65",
    GITHUB_REPO: "gdps-bot",
    GITHUB_FILE: "custom-badges/badges.json",
    GDPS_URL: "https://fhgdps.com"
};

async function resolveGDPSAccount(username) {
    const body = new URLSearchParams({ str: username, total: "0", page: "0", secret: "Wmfd2893gb7" });
    const res = await fetch(`${CONFIG.GDPS_URL}/getGJUsers20.php`, {
        method: "POST",
        headers: { "User-Agent": "", "Content-Type": "application/x-www-form-urlencoded" },
        body: body.toString()
    });
    const text = await res.text();
    if (!text || text === "-1") return null;

    for (const u of text.split("#").split("|")) {
        const parts = u.split(":");
        const map = {};
        for (let i = 0; i < parts.length - 1; i += 2) map[parts[i]] = parts[i + 1];
        if (map["1"] && map["1"].toLowerCase() === username.toLowerCase() && map["16"]) {
            return { username: map["1"], accountId: String(map["16"]) };
        }
    }
    return null;
}

async function getGitHubBadges() {
    const url = `https://github.com{CONFIG.GITHUB_OWNER}/${CONFIG.GITHUB_REPO}/contents/${CONFIG.GITHUB_FILE}`;
    const res = await fetch(url, {
        headers: { "Authorization": `Bearer ${CONFIG.GITHUB_TOKEN}`, "Accept": "application/vnd.github+json" }
    });
    const data = await res.json();
    return { badges: JSON.parse(Buffer.from(data.content, 'base64').toString('utf8')), sha: data.sha };
}

async function saveGitHubBadges(badgesObj, sha, message) {
    const url = `https://github.com{CONFIG.GITHUB_OWNER}/${CONFIG.GITHUB_REPO}/contents/${CONFIG.GITHUB_FILE}`;
    await fetch(url, {
        method: "PUT",
        headers: {
            "Authorization": `Bearer ${CONFIG.GITHUB_TOKEN}`,
            "Accept": "application/vnd.github+json",
            "Content-Type": "application/json"
        },
        body: JSON.stringify({
            message,
            content: Buffer.from(JSON.stringify(badgesObj, null, 2)).toString('base64'),
            sha
        })
    });
}
const commands = [
    new SlashCommandBuilder()
        .setName('badge')
        .setDescription('Manage GDPS custom badges')
        .setDefaultMemberPermissions(PermissionFlagsBits.ManageRoles)
        .addSubcommand(s => s.setName('add').setDescription('Give a badge by username')
            .addStringOption(o => o.setName('username').setDescription('GDPS Username').setRequired(true))
            .addIntegerOption(o => o.setName('badge').setDescription('Badge').setRequired(true).setAutocomplete(true)))
        .addSubcommand(s => s.setName('remove').setDescription('Remove a badge by username')
            .addStringOption(o => o.setName('username').setDescription('GDPS Username').setRequired(true))
            .addIntegerOption(o => o.setName('badge').setDescription('Badge').setRequired(true).setAutocomplete(true)))
        .addSubcommand(s => s.setName('clear').setDescription('Clear all badges by username')
            .addStringOption(o => o.setName('username').setDescription('GDPS Username').setRequired(true)))
        .addSubcommand(s => s.setName('view').setDescription('View badges by username')
            .addStringOption(o => o.setName('username').setDescription('GDPS Username').setRequired(true)))
].map(c => c.toJSON());

const client = new Client({ intents: [GatewayIntentBits.Guilds] });

client.once('ready', async () => {
    const rest = new REST({ version: '10' }).setToken(CONFIG.DISCORD_TOKEN);
    await rest.put(Routes.applicationGuildCommands(CONFIG.CLIENT_ID, CONFIG.GUILD_ID), { body: commands });
    console.log(`Logged in as ${client.user.tag}!`);
});

client.on('interactionCreate', async interaction => {
    if (interaction.isAutocomplete()) {
        const q = interaction.options.getFocused().toLowerCase();
        const matches = BADGES.filter(b => b.name.toLowerCase().includes(q) || String(b.id) === q).slice(0, 25);
        return interaction.respond(matches.map(b => ({ name: `${b.id}. ${b.name}`, value: b.id })));
    }
    if (!interaction.isChatInputCommand() || interaction.commandName !== 'badge') return;

    await interaction.deferReply({ ephemeral: true });
    const sub = interaction.options.getSubcommand();
    const player = await resolveGDPSAccount(interaction.options.getString('username').trim());
    if (!player) return interaction.editReply("Could not find that GDPS username!");

    const { badges, sha } = await getGitHubBadges();
    if (sub === 'view') {
        const list = badges[player.accountId] || [];
        const names = list.map(id => BADGES.find(b => b.id === id)?.name || id).join(', ') || "None";
        return interaction.editReply(`**${player.username}** (ID: ${player.accountId}): ${names}`);
    }
    if (sub === 'clear') {
        delete badges[player.accountId];
        await saveGitHubBadges(badges, sha, `Clear badges for ${player.username}`);
        return interaction.editReply(`Cleared all badges for **${player.username}**.`);
    }

    const badgeId = interaction.options.getInteger('badge');
    const badgeInfo = BADGES.find(b => b.id === badgeId);
    if (!badgeInfo) return interaction.editReply("Invalid badge.");

    badges[player.accountId] = badges[player.accountId] || [];
    if (sub === 'add') {
        if (!badges[player.accountId].includes(badgeId)) badges[player.accountId].push(badgeId);
        await saveGitHubBadges(badges, sha, `Add ${badgeInfo.name} to ${player.username}`);
        return interaction.editReply(`Added **${badgeInfo.name}** to **${player.username}**!`);
    }
    if (sub === 'remove') {
        badges[player.accountId] = badges[player.accountId].filter(id => id !== badgeId);
        if (badges[player.accountId].length === 0) delete badges[player.accountId];
        await saveGitHubBadges(badges, sha, `Remove ${badgeInfo.name} from ${player.username}`);
        return interaction.editReply(`Removed **${badgeInfo.name}** from **${player.username}**!`);
    }
});

client.login(CONFIG.DISCORD_TOKEN);
