#include "common.c"

void printfLoginMenu(int socketFd)
{
    iprintf(socketFd, "\nBook Management service\n");
    iprintf(socketFd, "1. Login\n");
    iprintf(socketFd, "2. 신규회원 가입\n");
    iprintf(socketFd, "0. Quit\n");
    iprintf(socketFd, "Choose num : ");
}

member getMemberInfo(int memberFd, int key)
{
    member record;
    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(record);
    lock.l_start = key * sizeof(record) + sizeof(int);
    lock.l_type = F_RDLCK;
    fcntl(memberFd, F_SETLKW, &lock);
    lseek(memberFd, sizeof(int) + sizeof(record) * key, SEEK_SET);
    int check = read(memberFd, (char *)&record, sizeof(record));
    if (check < 1)
    {
        lock.l_type = F_UNLCK;
        fcntl(memberFd, F_SETLK, &lock);
        record.key = NO_DATA;
        return record;
    }
    lock.l_type = F_UNLCK;
    fcntl(memberFd, F_SETLK, &lock);
    return record;
}

int getMemberHeader(int memberFd)
{
    int count = 0;
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(int);
    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    fcntl(memberFd, F_SETLKW, &lock);
    lseek(memberFd, 0, SEEK_SET);
    read(memberFd, &count, sizeof(int));
    lock.l_type = F_WRLCK;
    fcntl(memberFd, F_SETLKW, &lock);
    lock.l_type = F_UNLCK;
    fcntl(memberFd, F_SETLKW, &lock);
    return count;
}

void upMemberHeader(int memberFd)
{
    int count = 0;
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(int);
    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    fcntl(memberFd, F_SETLKW, &lock);
    lseek(memberFd, 0, SEEK_SET);
    read(memberFd, &count, sizeof(int));
    count++;
    lseek(memberFd, 0, SEEK_SET);
    write(memberFd, &count, sizeof(int));
    lock.l_type = F_UNLCK;
    fcntl(memberFd, F_SETLKW, &lock);
}

void addMemberInfo(int socketFd, int memberFd)
{
    member record;
    char temp[30];
    char Y;
    int key = getMemberHeader(memberFd);
    record.key = key;
    iprintf(socketFd, "Book Management service\n");
    iprintf(socketFd, "<2. 신규회원가입>\n");
    iprintf(socketFd, "id : ");
    iscanf(socketFd, temp);
    strcpy(record.id, temp);
    int i = 0;
    while (1)
    {
        member temp = getMemberInfo(memberFd, i);
        if (temp.key == NO_DATA)
            break;
        if (temp.key != 0 && strcmp(record.id, temp.id) == 0)
        {
            iprintf(socketFd, "이미 존재하는 아이디입니다.\n");
            return;
        }
        i++;
    }
    iprintf(socketFd, "pass : ");
    iscanf(socketFd, temp);
    strcpy(record.password, temp);
    iprintf(socketFd, "회원 이름 : ");
    iscanf(socketFd, temp);
    strcpy(record.name, temp);
    iprintf(socketFd, "휴대폰 번호 : ");
    iscanf(socketFd, temp);
    strcpy(record.phoneNum, temp);
    iprintf(socketFd, "이메일 : ");
    iscanf(socketFd, temp);
    strcpy(record.email, temp);
    iprintf(socketFd, "생년월일(YYMMDD) : ");
    iscanf(socketFd, temp);
    strcpy(record.birth, temp);
    iprintf(socketFd, "admin 여부(Y/N) : ");
    iscanf(socketFd, temp);
    if (strcmp(temp, "Y") == 0)
        record.admin = 1;
    else
        record.admin = 0;

    upMemberHeader(memberFd);
    lseek(memberFd, 0, SEEK_END);
    write(memberFd, (char *)&record, sizeof(record));
}

member login(int socketFd, int memberFd)
{
    member record;
    char id[30];
    char password[30];
    int count = 0;
    while (count < 3)
    {
        iprintf(socketFd, "Book Management Service\n");
        iprintf(socketFd, "id와 password를 입력하세요\n");
        iprintf(socketFd, "id : ");
        iscanf(socketFd, id);
        iprintf(socketFd, "password : ");
        iscanf(socketFd, password);

        int i = 0;
        while (1)
        {
            record = getMemberInfo(memberFd, i);
            if (record.key == NO_DATA)
                break;
            if (record.key != 0)
            {
                if (strcmp(record.id, id) == 0 && strcmp(record.password, password) == 0)
                {
                    return record;
                }
            }
            i++;
        }
        iprintf(socketFd, "아이디 또는 비밀번호가 잘못되었습니다.\n");
        count++;
    }
    record.key = NO_DATA;
    return record;
}

member loginMenu(int socketFd, int memberFd)
{
    member record;
    record.key = NO_DATA;
    int sel = 0;
    while (record.key == NO_DATA)
    {
        printfLoginMenu(socketFd);
        iscanfInt(socketFd, &sel);
        if (sel == 0)
            record.key = -300;
        else if (sel == 1)
        {
            record = login(socketFd, memberFd);
            nowtime(socketFd);
            printf("%s님이 로그인 했습니다.\n", record.id);
        }
        else if (sel == 2)
            addMemberInfo(socketFd, memberFd);
    }

    return record;
}