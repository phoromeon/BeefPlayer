Beef Player
开发环境：QtCreator
使用QT版本6.9.0+
所使用到的库：libgme
libgme的获取途径，可通过github仓库获取
git clone https://github.com/libgme/game-music-emu.git
通过clone的源代码的README文件编译libgme库。
已将libgme库的部分操作封装成GmeDecoder类，可扩展。
已实现播放、暂停、单首切换等基本功能。
虽然libgme库支持多个平台的音乐文件，但本程序只面向nsf/gbs等含多曲目格式。
版本号为0.95,基本可用。
