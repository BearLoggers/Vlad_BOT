const Discord = require('discord.js');
const client = new Discord.Client();
const fs = require('fs');

const Nightmare = require('nightmare');
const Iconv = require('iconv').Iconv;

const fetchOptions = {
    headers: {
        cookie: 'kvk-user=%7B%22vkToken%22%3A%22c66e33f9d9df2d44135fafdea4f58f822b5af5c514e0d68b509e8e2a6855b7e38e178064d34fb75518ddf%22%2C%22isGuest%22%3Afalse%2C%22initialized%22%3Afalse%2C%22id%22%3A%22585381466%22%7D'
    }
};

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
if (!fs.existsSync("vkquery.txt")) fs.writeFileSync("vkquery.txt", "");

// Сырая ссылка
fs.watchFile("link.txt", (curr, prev) => {
    console.log("link был изменен");

    const link = fs.readFileSync("link.txt", "utf8");
    console.log(`Прочитанный линк: '${link}'`);
    if (link) {
        playExternal(link);
    }
});


// Поиск по вк
fs.watchFile("vkquery.txt", (curr, prev) => {
    console.log("vkquery был изменен");

    let buffer = fs.readFileSync("vkquery.txt");
    // console.log("buffer", buffer);
    let converter = Iconv('CP866', 'UTF-8');
    let converted = converter.convert(buffer);
    // console.log("converted", converted);

    let query = encodeURI(converted);
    console.log(`Прочитанный query: '${query}'`);
    if (query) {
        const myMusicButton = 'body > div.main-container.pt-3 > div:nth-child(3) > div:nth-child(2) > div.btn-group > button.btn.btn-outline-primary';
        // const firstDownloadLink = 'body > div.main-container.pt-3 > div:nth-child(3) > table > tbody > tr:nth-child(1) > td.align-middle.pr-0 > a';
        const nightmare = Nightmare({
            show: false
        });

        nightmare
            .cookies.set({
                url: 'http://kissvk.com/',
                name: 'kvk-user',
                value: '%7B%22vkToken%22%3A%22c66e33f9d9df2d44135fafdea4f58f822b5af5c514e0d68b509e8e2a6855b7e38e178064d34fb75518ddf%22%2C%22isGuest%22%3Afalse%2C%22initialized%22%3Afalse%2C%22id%22%3A%22585381466%22%7D',
            })
            .goto(`http://kissvk.com/?search=${query}`)
            .wait(myMusicButton)
            .evaluate(() => {
                const a = document.querySelector('body > div.main-container.pt-3 > div:nth-child(3) > table > tbody > tr:nth-child(1) > td.align-middle.pr-0 > a');
                if (a) return a.href;
                else return null;
            })
            .end()
            .then(url => {
                if (url && url.length > 7 && url.startsWith('http')) {
                    playExternal(url);

                    let startIndex = url.indexOf("artist=") + "artist=".length;
                    const artist = decodeURI(url.substring(startIndex, url.indexOf('&', startIndex)))
                                    .replace(/%2C/gi, "");

                    startIndex = url.indexOf("title=") + "title=".length;
                    const title = decodeURI(url.substring(startIndex, url.indexOf('&', startIndex)))
                                    .replace(/%2C/gi, "");

                    console.log(artist + " - " + title);

                    fs.writeFileSync("../CPP_SERVER/CPP_SERVER/vksearch.status", `success\n${artist} - ${title}\n`);
                } else {
                    console.log("Что-то не то с ссылкой");

                    fs.writeFileSync("../CPP_SERVER/CPP_SERVER/vksearch.status", 'failed');
                }
            })
            .catch(error => {
                console.error('KISSVK failed:', error);

                fs.writeFileSync("../CPP_SERVER/CPP_SERVER/vksearch.status", 'failed');
            });
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

    } else {
        console.log("Упс! Канал не инициализирован.\n");
    }
}

client.on('message', msg => {
    if (msg.content === 'ping') {
        msg.reply(`pong ${new Date()}`);
    } else if (msg.content === 'влад готовит хуёво') {
        msg.reply('Да бля');
    } else if (msg.content === 'подошёл') {
        console.log('Меня позвали, ', msg.member.voice.channel);
        if (msg.member.voice.channel) {
            msg.member.voice.channel.join()
                .then(connection => {
                    connection.play('https://i15.kissvk.com/api/song/download/get/10/Avicii-The%20Nights-kissvk.com.mp3?origin=kissvk.com&url=sid%3A%2F%2F585381466_456239017_6a9b9a849ef2603cde_bdbb0f68b93fe28dc7&artist=Avicii&title=The%20Nights&index=0&future_urls=');
                }).catch(err => {
                    console.log(err);
                    msg.reply('Маслина = словлена, ', err);
                });

        } else {
            msg.reply('адрес скажи сначала');
        }
    } else if (msg.content === 'ушёл') {
        if (msg.member.voice.channel)
            msg.member.voice.channel.leave();
    }
});

client.login(fs.readFileSync('.token', 'utf8'));