#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/un.h>

#define ADDRESS "/tmp/UNIX_sockets/socket" //адрес для связи

//демон для обработки сообщений от клиента

/**
 * демон создаст папку /tmp/UNIX_sockets/
 * в которой будет лежать сокет "socket", который удалится после завершения сианса работы,
 * и файл log.log, в котором лежат логи
 */
void server ();
int init_daemon(void)
{
    int pid;
    int i;

    // Игнорируем сигнал ввода-вывода терминала, сигнал STOP
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGHUP,SIG_IGN);

    pid = fork();
    if(pid > 0) {
            exit(0); // Завершаем родительский процесс, делая дочерний процесс фоновым процессом
    }
    else if(pid < 0) {
            return -1;
    }

    // Создаем новую группу процессов, в этой новой группе процессов дочерний процесс становится первым процессом этой группы процессов, так что процесс отделен от всех терминалов
    setsid();

    // Создаем новый дочерний процесс снова, выходим из родительского процесса, убеждаемся, что процесс не является лидером процесса, и делаем процесс неспособным открыть новый терминал
    pid=fork();
    if( pid > 0) {
            exit(0);
    }
    else if( pid< 0) {
            return -1;
    }

    // Закрываем все файловые дескрипторы, унаследованные от родительского процесса, которые больше не нужны
    for(i=0;i< NOFILE;close(i++));

    // Изменяем рабочий каталог, чтобы процесс не связывался ни с одной файловой системой
    chdir("/");

    // Устанавливаем слово маски создания файла на 0
    umask(0);

    // Игнорируем сигнал SIGCHLD
    signal(SIGCHLD,SIG_IGN);

    //запускаем сервер для общения с клиентом
    server();

    return 0;
}

//метод дя общения с клиентами
void server ()
{
    int d, d1, len, ca_len;
    FILE *fp;
    FILE *log;
    struct sockaddr_un sa, ca;

    //создаем папку и log.log в ней
    system("mkdir -p /tmp/UNIX_sockets");
    char path[] = "/tmp/UNIX_sockets/log.log";
    log = fopen(path, "a");

    char* message[256];//сообщение для отправки

    strcpy(message,"server: init");

    logging(log, message);//метод для записи сообщения в логи

    //получаем свой сокет-дескриптор:
    if((d = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
     perror ("client: socket"); exit (1);
    }

    //создаем адрес, c которым будут связываться клиенты
    sa.sun_family = AF_UNIX;
    strcpy (sa.sun_path, ADDRESS);
    /*  связываем адрес с сокетом;
        уничтожаем файл с именем ADDRESS, если он существует,
        для того, чтобы вызов bind завершился успешно
    */
    unlink (ADDRESS);
    len = sizeof ( sa.sun_family) + strlen (sa.sun_path);
    if ( bind ( d, &sa, len) < 0 ) {
            perror ("server: bind"); exit (1);
    }
    //слушаем запросы на сокет
    if ( listen ( d, 5) < 0 ) {
            perror ("server: listen"); exit (1);
    }
    //связываемся с клиентом через неименованный сокет с дескриптором d1:

    if (( d1 = accept ( d, &ca, &ca_len)) < 0 ) {
            perror ("server: accept"); exit (1);
    }

    //пишем клиенту:
    strcpy(message,"server: connect, please send the word (to exit, enter 'exit')");
    send (d1, message,sizeof(message), 0);

    logging(log, message);
    char* str[256];

    //принимем сообщение от клиента и записываем сообщения в логи
    while(1)
    {
        if(recv(d1, str, sizeof(str), 0) < 0 )
        {
           perror ("server: message"); exit (1);
        }
        logging(log, str);

        if(!strcmp(str,"exit")) {
            break;
        }
    }

    //закрываем все и удаляем сокет
    close (d1);
    close (log);
    remove (ADDRESS);

    exit (0);
}

//метод для записи сообщения в логи
void logging(FILE *log, char* str[])
{
    time_t now;
    time(&now);
    fprintf(log,"%s message: %5s\n",ctime(&now),str);
}
int main()
{
    init_daemon();
}

