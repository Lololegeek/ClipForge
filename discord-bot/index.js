// Bot ClipForge : message de bienvenue et rôle Membre à l'arrivée, message de départ, tickets, vocaux temporaires.
import {
  ActionRowBuilder, ActivityType, ButtonBuilder, ButtonStyle, ChannelType,
  Client, EmbedBuilder, Events, GatewayIntentBits, PermissionFlagsBits,
} from "discord.js";

const {
  DISCORD_TOKEN,
  WELCOME_CHANNEL_ID = "1565354316091363439",
  RULES_CHANNEL_ID = "1565138025779044432",
  HELP_CHANNEL_ID = "1565139361153032273",
  GOODBYE_CHANNEL_ID,
  MEMBER_ROLE_NAME = "💬 Membre",
} = process.env;

if (!DISCORD_TOKEN) {
  console.error("DISCORD_TOKEN manquant");
  process.exit(1);
}

const client = new Client({
  intents: [GatewayIntentBits.Guilds, GatewayIntentBits.GuildMembers, GatewayIntentBits.GuildVoiceStates],
});

client.once(Events.ClientReady, (c) => {
  c.user.setActivity("tes meilleurs clips", { type: ActivityType.Watching });
  console.log(`Connecté en tant que ${c.user.tag}`);
});

client.on(Events.GuildMemberAdd, async (member) => {
  if (member.user.bot) return;

  const role = member.guild.roles.cache.find((r) => r.name === MEMBER_ROLE_NAME);
  if (role) await member.roles.add(role).catch((e) => console.error("rôle", e.message));

  const channel = await member.guild.channels.fetch(WELCOME_CHANNEL_ID).catch(() => null);
  if (!channel?.isTextBased()) return;

  const embed = new EmbedBuilder()
    .setColor(8150271)
    .setAuthor({ name: "Nouveau membre", iconURL: member.guild.iconURL() ?? undefined })
    .setTitle(`👋 Bienvenue ${member.displayName} !`)
    .setDescription(
      `Content de te voir sur le serveur **ClipForge**, ${member}.\n\n` +
      `📜 Lis les règles dans <#${RULES_CHANNEL_ID}>\n` +
      `🆘 Une question ? Passe dans <#${HELP_CHANNEL_ID}>\n` +
      `🎬 Et surtout, partage tes meilleurs clips !`,
    )
    .setThumbnail(member.user.displayAvatarURL({ size: 256 }))
    .setFooter({ text: `Tu es le membre n°${member.guild.memberCount}` })
    .setTimestamp();

  const buttons = new ActionRowBuilder().addComponents(
    new ButtonBuilder()
      .setStyle(ButtonStyle.Link)
      .setLabel("Télécharger ClipForge")
      .setEmoji("⬇️")
      .setURL("https://lololegeek.github.io/ClipForge/telechargement/"),
  );

  await channel
    .send({ content: `${member}`, embeds: [embed], components: [buttons] })
    .catch((e) => console.error("bienvenue", e.message));
});

client.on(Events.GuildMemberRemove, async (member) => {
  if (member.user.bot) return;

  const channel = GOODBYE_CHANNEL_ID
    ? await member.guild.channels.fetch(GOODBYE_CHANNEL_ID).catch(() => null)
    : member.guild.channels.cache.find((c) => c.name.endsWith("au-revoir"));
  if (!channel?.isTextBased()) return;

  const embed = new EmbedBuilder()
    .setColor(8150271)
    .setDescription(`🚪 **${member.user.username}** a quitté le serveur. À bientôt !`)
    .setFooter({ text: `Nous sommes maintenant ${member.guild.memberCount}` })
    .setTimestamp();

  await channel.send({ embeds: [embed] }).catch((e) => console.error("au revoir", e.message));
});

// Tickets : un panneau avec un bouton dans le salon tickets, un salon privé par demande.
const findTicketPanelChannel = (guild) =>
  guild.channels.cache.find((c) => c.type === ChannelType.GuildText && c.name.endsWith("tickets"));

client.on(Events.ClientReady, async (c) => {
  for (const guild of c.guilds.cache.values()) {
    await guild.channels.fetch();
    const channel = findTicketPanelChannel(guild);
    if (!channel) continue;
    const recent = await channel.messages.fetch({ limit: 20 }).catch(() => null);
    if (recent?.some((m) => m.author.id === c.user.id)) continue;

    const embed = new EmbedBuilder()
      .setColor(8150271)
      .setTitle("🎫 Contacter le support")
      .setDescription(
        "Un souci avec ClipForge, une question privée ou un membre à signaler ?\n\n" +
        "Clique sur le bouton : un salon privé s'ouvre, visible seulement par toi et l'équipe.\n" +
        `Pour les questions générales, <#${HELP_CHANNEL_ID}> reste le plus rapide.`,
      );
    const row = new ActionRowBuilder().addComponents(
      new ButtonBuilder().setCustomId("ticket:open").setStyle(ButtonStyle.Primary)
        .setLabel("Ouvrir un ticket").setEmoji("🎫"),
    );
    await channel.send({ embeds: [embed], components: [row] })
      .catch((e) => console.error("panneau tickets", e.message));
  }
});

client.on(Events.InteractionCreate, async (interaction) => {
  if (!interaction.isButton() || !interaction.inGuild()) return;
  const { guild, user } = interaction;

  if (interaction.customId === "ticket:open") {
    const name = `ticket-${user.username}`.toLowerCase().replace(/[^a-z0-9-]/g, "").slice(0, 90);
    const existing = guild.channels.cache.find((c) => c.topic === `ticket:${user.id}`);
    if (existing) {
      return interaction.reply({ content: `Tu as déjà un ticket ouvert : ${existing}`, ephemeral: true });
    }

    const panel = interaction.channel;
    const staffRoles = guild.roles.cache.filter((r) =>
      r.permissions.has(PermissionFlagsBits.ManageMessages) && !r.managed);
    const channel = await guild.channels.create({
      name,
      type: ChannelType.GuildText,
      parent: panel.parentId,
      topic: `ticket:${user.id}`,
      permissionOverwrites: [
        { id: guild.roles.everyone.id, deny: [PermissionFlagsBits.ViewChannel] },
        { id: user.id, allow: [PermissionFlagsBits.ViewChannel, PermissionFlagsBits.SendMessages,
          PermissionFlagsBits.AttachFiles, PermissionFlagsBits.ReadMessageHistory] },
        { id: client.user.id, allow: [PermissionFlagsBits.ViewChannel, PermissionFlagsBits.SendMessages,
          PermissionFlagsBits.ManageChannels] },
        ...staffRoles.map((r) => ({ id: r.id, allow: [PermissionFlagsBits.ViewChannel,
          PermissionFlagsBits.SendMessages, PermissionFlagsBits.ReadMessageHistory] })),
      ],
    }).catch((e) => console.error("ticket", e.message));
    if (!channel) {
      return interaction.reply({ content: "Impossible d'ouvrir le ticket, préviens l'équipe.", ephemeral: true });
    }

    const embed = new EmbedBuilder()
      .setColor(8150271)
      .setTitle("🎫 Ticket ouvert")
      .setDescription(
        `Salut ${user} ! Explique ton souci ici, l'équipe te répond dès que possible.\n\n` +
        "Pour un bug : version de ClipForge, version de Windows, jeu concerné et étapes pour reproduire.",
      );
    const row = new ActionRowBuilder().addComponents(
      new ButtonBuilder().setCustomId("ticket:close").setStyle(ButtonStyle.Danger)
        .setLabel("Fermer le ticket").setEmoji("🔒"),
    );
    await channel.send({ content: `${user}`, embeds: [embed], components: [row] });
    return interaction.reply({ content: `Ton ticket est ouvert : ${channel}`, ephemeral: true });
  }

  if (interaction.customId === "ticket:close") {
    if (!interaction.channel?.topic?.startsWith("ticket:")) return;
    await interaction.reply("🔒 Ticket fermé, le salon sera supprimé dans 5 secondes.");
    setTimeout(() => interaction.channel.delete().catch((e) => console.error("fermeture", e.message)), 5000);
  }
});

// Vocaux temporaires : rejoindre « Créer ton salon » crée un vocal perso, supprimé quand il se vide.
const tempVoiceChannels = new Set();

client.on(Events.VoiceStateUpdate, async (oldState, newState) => {
  const left = oldState.channel;
  if (left && tempVoiceChannels.has(left.id) && left.members.size === 0) {
    tempVoiceChannels.delete(left.id);
    await left.delete().catch((e) => console.error("vocal temporaire", e.message));
  }

  const joined = newState.channel;
  if (!joined || !joined.name.toLowerCase().includes("créer ton salon")) return;
  const member = newState.member;
  const channel = await newState.guild.channels.create({
    name: `🔊・Salon de ${member.displayName}`,
    type: ChannelType.GuildVoice,
    parent: joined.parentId,
    permissionOverwrites: [
      { id: member.id, allow: [PermissionFlagsBits.ManageChannels, PermissionFlagsBits.MoveMembers,
        PermissionFlagsBits.MuteMembers] },
    ],
  }).catch((e) => console.error("vocal temporaire", e.message));
  if (!channel) return;
  tempVoiceChannels.add(channel.id);
  await member.voice.setChannel(channel).catch(() => channel.delete().catch(() => {}));
});

client.login(DISCORD_TOKEN);
