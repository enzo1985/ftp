FTP 客户端 - 服务器实现
===========
这是一个文件传输程序的简单实现，它包含了定制的客户端和服务器程序，用来提供授权用户，列出远程文件目录，以及获取远程文件的功能。

源代码结构:
ftp/
    client/
        ftclient.c
        ftclient.h
        makefile
    common/
        common.c
        common.h
    server/
        ftserve.c
        ftserve.h
        makefile
        .auth
    readme_pic/
        pic_1.jpg
        pic_2.jpg
        pic_3.jpg
        pic_4.jpg


用法
编译链接ftp服务器:
```
$ cd server/
$ make
```

编译链接ftp客户端:
```
$ cd client/
$ make
```

运行ftp服务器:
```
$ server/ftserve PORTNO
```

运行ftp客户端:
```
$ client/ftclient HOSTNAME PORTNO

指令:
    list
    get <文件名>
    quit
```

可用指令:
```
list            - 获取当前远程目录的文件列表
get <filename>  - 获取特定文件
quit            - 终止当前ftp会话
```

登录:
```
Name: anonymous
Password: [empty]
```
服务器架构:
![pic1](https://raw.githubusercontent.com/tantao0675/ftp/master/readme_pic/pic_2.jpg)
![pic1](https://raw.githubusercontent.com/tantao0675/ftp/master/readme_pic/pic_3.jpg)
![pic1](https://raw.githubusercontent.com/tantao0675/ftp/master/readme_pic/pic_4.jpg)

客户端架构:
wait
