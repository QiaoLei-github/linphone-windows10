# linphone-windows10
windows10下编译linphone-sdk  visual stadio不要超过2017
linphone-sdk使用git、GetGnuWin32、mingw、cmake、python、pip、yasm、nasm、doxygen、Pystache、six、wheel、graphviz、perl、qt工具   我不使用java所以jdk没有安装
下载地址
1.cmake  https://cmake.org/download/  下载windows版
2.python  https://www.python.org/downloads/release/python-381/  下载windows版，我是用 2.7.17版 添加环境变量到系统 -----使用pip -V查看pip版本号，如果python中没有需要手动安装，
https://pypi.python.org/pypi/pip#downloads 下载好后进入目录使用python setup.py install安装
3.yasm  https://yasm.tortall.net/Download.html 对应下载的是x86或者x64，将yasm.exe放入Python\Scripts下
4.pystache  pip install pystache
5.six  pip install six
6.wheel pip intall wheel
7.graphviz https://graphviz.gitlab.io/_pages/Download/Download_windows.html解压后将bin添加环境变量
8.doxygen https://www.doxygen.nl/download.html解压后将bin添加环境变量
9.perl http://strawberryperl.com/
10.git https://gitforwindows.org/
11.getgnuwin32 https://sourceforge.net/projects/getgnuwin32/files/latest/download解压后将bin添加环境变量
12.mingw  https://sourceforge.net/projects/mingw/?source=typ_redirect  --linphone默认使用mingw路径为C:\MinGW ，选择安装 mingw32-base、mingw32-gcc-g++   如果要使用mingw
编译ffmpeg、openh264、vpx还需要下载pkg-config https://download.gnome.org/binaries/解压后将bin目录下的 pkg-config.exe 拷贝到目录 MinGW\msys\1.0\bin 中、
glib https://ftp.acc.umu.se/pub/GNOME/binaries/解压后将bin目录下的 libglib-2.0-0.dll 拷贝到目录 C:\MinGW\msys\1.0\bin 中、SDL2  https://www.libsdl.org/release/解压后将
i686-w64-mingw32或x86_64-w64-mingw32目录下的 bin、include、lib、share 的全部内容拷贝到C:\MinGW\msys\1.0\ 对应的目录中，修改 C:\MinGW\msys\1.0\bin\sdl2-config 文件，prefix 
改为 /c/MinGW/msys/1.0/bin 对应 C:\MinGW\msys\1.0\bin,修改 C:\MinGW\msys\1.0\lib\pkgconfig\sdl2.pc 文件，prefix 改为 /c/MinGW/msys/1.0/bin 对应 C:\MinGW\msys\1.0\bin。
将yasm.exe、nasm.exe 放到MinGW\msys\1.0\bin下。
13.qt https://www.qt.io/download下载免费的qt版本要大于等于5.9 安装好添加环境变量 新建Qt5_DIR="C:\Qt\5.9\msvc2015\lib\cmake"  添加PATH="C:\Qt\5.9\msvc2015\bin;"
14.安装vs2017  选择.net桌面开发 通用windows平台开发 使用c++桌面开发 ---windows8.1 sdk和ucrt sdk、用于cmake的visual c++工具、windows10 sdk选中安装
