const Discord = require('discord.js');
const client = new Discord.Client();
const fs = require('fs');

const voiceChannelID = '623169566641618957';
let selectedVoiceChannel = null;

client.on('ready', () => {
    console.log(`Залогинен как ${client.user.tag}!`);
    // console.log(`Доступные каналы: `, client.channels);

    client.channels.fetch(voiceChannelID)
        .then(channel => {
            console.log(`Избранный канал: `, channel.name);
            selectedVoiceChannel = channel;
        });
});

if (!fs.existsSync("link.txt")) fs.writeFileSync("link.txt", "");

fs.watchFile("link.txt", (curr, prev) => {
    console.log("link был изменен");

    const link = fs.readFileSync("link.txt", "utf8");
    console.log(`Прочитанный линк: '${link}'`);
    if (link) {
        playExternal(link);
    }
});

function playExternal(link) {
    if (selectedVoiceChannel) {
        selectedVoiceChannel.join()
            .then(connection => {
                connection.play(link);
            }).catch(err => {
                console.log(err);
                // msg.reply('Маслина = словлена, ', err);
            });

    }
    else {
        console.log("Упс! Канал не инициализирован.\n");
    }
}

client.on('message', msg => {
    if (msg.content === 'ping') {
        msg.reply(`pong ${new Date()}`);
    }
    else if (msg.content === 'влад готовит хуёво') {
        msg.reply('Да бля');
    }
    else if (msg.content === 'подошёл') {
        console.log('Меня позвали, ', msg.member.voice.channel);
        if (msg.member.voice.channel) {
            msg.member.voice.channel.join()
                .then(connection => {
                    connection.play('https://i15.kissvk.com/api/song/download/get/10/Avicii-The%20Nights-kissvk.com.mp3?origin=kissvk.com&url=sid%3A%2F%2F585381466_456239017_6a9b9a849ef2603cde_bdbb0f68b93fe28dc7&artist=Avicii&title=The%20Nights&index=0&future_urls=');
                }).catch(err => {
                    console.log(err);
                    msg.reply('Маслина = словлена, ', err);
                });

        }
        else {
            msg.reply('адрес скажи сначала');		
        }
    }
    else if (msg.content === 'ушёл') {
        if (msg.member.voice.channel)
            msg.member.voice.channel.leave();
    }
});

client.login(fs.readFileSync('.token', 'utf8'));