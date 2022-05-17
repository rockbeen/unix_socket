#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#define ADDRESS "/tmp/UNIX_sockets/socket" //адрес для связи

//клиент-служит для отправки сообщений демону

/**
 * демон создаст папку /tmp/UNIX_sockets/
 * в которой будет лежать сокет "socket", который удалится после завершения сианса работы,
 * и файл log.log, в котором лежат логи
 */
void main ()
{
    int  s, len;
    char* str[256];//сообщение
    FILE *fp;
    struct sockaddr_un sa;
    //получаем свой сокет-дескриптор:
    if ((s = socket (AF_UNIX, SOCK_STREAM, 0))<0)
    {
    perror ("client: socket"); exit(1);
    }
    //создаем адрес, по которому будем связываться с сервером:
    sa.sun_family = AF_UNIX;
    strcpy (sa.sun_path, ADDRESS);

    //пытаемся связаться с сервером:
    len = sizeof ( sa.sun_family) + strlen ( sa.sun_path);
    if ( connect ( s, &sa, len) < 0 ){
            perror ("client: connect"); exit (1);
    }

    //читаем сообщения сервера
    fp = fdopen (s, "r");

    if(recv(s, str, sizeof(str), 0) < 0 )
    {
        perror ("client: message"); exit (1);
    }

    //выводим информацию от сервера
    printf("%5s\n",str);

    //продолжаем диалог с сервером, пока не будет введенно слово exit
    do{
        scanf("%5s",str);
        send (s, str,  sizeof(str), 0);

    } while(strcmp(str,"exit"));

    //завершаем сеанс работы
    close (s);
    exit (0);
}
