const Discord = require('discord.js');
const bot = new Discord.Client();

bot.on('message', (message) => {
    if(message.content =='ping'){
        //message.reply('pong'); //replies '@person, pong'
        message.channel.send('pong'); //sends message on channel
    }
});

bot.login('NzQ5NDQ2OTM2NTYxNTgyMTIx.X0sG2Q.-I78uSrzQXVI9ETOv1CpubcUUwk');
