#include "login.c"

void printBookMenu(int sockFd)
{
    iprintf(sockFd, "Book Management\n");
    iprintf(sockFd, "<일반 회원 도서관리>\n");
    iprintf(sockFd, "1. List up All Book(Sort by 도서명)\n");
    iprintf(sockFd, "2. List up All Book(Sort by 가격)\n");
    iprintf(sockFd, "3. Add a New book\n");
    iprintf(sockFd, "4. Update a Book\n");
    iprintf(sockFd, "5. Remove a Book\n");
    iprintf(sockFd, "6. 도서명으로 책검색(minimum 2char)\n");
    iprintf(sockFd, "7. 저자명으로 책검색\n");
    iprintf(sockFd, "8. 개인정보 변경\n");
    iprintf(sockFd, "0. quit\n");
    iprintf(sockFd, "Choose num : ");
}

void printBookMenu_admin(int sockFd)
{
    iprintf(sockFd, "Book Management\n");
    iprintf(sockFd, "<admin 회원 도서관리>\n");
    iprintf(sockFd, "1. List up All Book(Sort by 도서명)\n");
    iprintf(sockFd, "2. List up All Book(Sort by 가격)\n");
    iprintf(sockFd, "3. Add a New book\n");
    iprintf(sockFd, "4. Update a Book\n");
    iprintf(sockFd, "5. Remove a Book\n");
    iprintf(sockFd, "6. 도서명으로 책검색(minimum 2char)\n");
    iprintf(sockFd, "7. 회원리스트보기\n");
    iprintf(sockFd, "8. 신규 회원정보 삽입\n");
    iprintf(sockFd, "9. 회원정보 삭제\n");
    iprintf(sockFd, "10. 회원정보 갱신\n");
    iprintf(sockFd, "11. 개인정보 변경\n");
    iprintf(sockFd, "0. quit\n");
    iprintf(sockFd, "Choose num : ");
}

book getBookInfo(int bookFd, int key)
{
    book record;
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(record);
    lock.l_start = key * sizeof(record) + sizeof(int);
    lock.l_type = F_RDLCK;

    fcntl(bookFd, F_SETLKW, &lock);
    lseek(bookFd, sizeof(int) + sizeof(record) * key, SEEK_SET);
    int check = read(bookFd, (char *)&record, sizeof(record));
    if (check < 1)
    {
        lock.l_type = F_UNLCK;
        fcntl(bookFd, F_SETLK, &lock);
        record.key = NO_DATA;
        return record;
    }
    lock.l_type = F_UNLCK;
    fcntl(bookFd, F_SETLK, &lock);
    return record;
}

void listupAllbook(int socketFd, int bookFd, member m)
{
    book record;
    iprintf(socketFd, "도서식별자 도서명 저자 출판년월일 가격 소유자회원id 추천리뷰\n");
    int i = 0;
    while (1)
    {
        record = getBookInfo(bookFd, i);
        if (record.key == NO_DATA)
        {
            break;
        }
        if (record.key != 0)
        {
            if (m.admin == 1 || strcmp(m.id, record.memberId) == 0)
            {
                iprintf(socketFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);
            }
        }
        i++;
    }
}

void setBookInfo(int sorcktFd, int bookFd, book record)
{
    struct flock lock;

    iprintf(sorcktFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(record);
    lock.l_start = record.key * sizeof(record) + sizeof(int);
    lock.l_type = F_RDLCK;

    fcntl(bookFd, F_SETLKW, &lock);
    lseek(bookFd, sizeof(int) + sizeof(record) * record.key, SEEK_SET);
    write(bookFd, (char *)&record, sizeof(record));

    lock.l_type = F_UNLCK;
    fcntl(bookFd, F_SETLK, &lock);
}

void upBookHeader(int bookFd)
{
    int count = 0;
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(int);
    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    fcntl(bookFd, F_SETLKW, &lock);
    lseek(bookFd, 0, SEEK_SET);
    read(bookFd, &count, sizeof(int));
    count++;
    lseek(bookFd, 0, SEEK_SET);
    write(bookFd, &count, sizeof(int));
    lock.l_type = F_UNLCK;
    fcntl(bookFd, F_SETLKW, &lock);
}

void downBookHeader(int bookFd)
{
    int count = 0;
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(int);
    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    fcntl(bookFd, F_SETLKW, &lock);
    lseek(bookFd, 0, SEEK_SET);
    read(bookFd, &count, sizeof(int));
    count--;
    lseek(bookFd, 0, SEEK_SET);
    write(bookFd, &count, sizeof(int));
    lock.l_type = F_UNLCK;
    fcntl(bookFd, F_SETLKW, &lock);
}

int getBookHeader(int bookFd)
{
    int count = 0;
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(int);
    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    fcntl(bookFd, F_SETLKW, &lock);
    lseek(bookFd, 0, SEEK_SET);
    read(bookFd, &count, sizeof(int));
    lock.l_type = F_WRLCK;
    fcntl(bookFd, F_SETLKW, &lock);
    lock.l_type = F_UNLCK;
    fcntl(bookFd, F_SETLKW, &lock);
    return count;
}

void addBookInfo(int socketFd, int bookFd, member m)
{
    book record;
    char temp[30];
    int tempInt = 0;
    iprintf(socketFd, " 도서식별자(0을제외한 자연수) : ");
    iscanfInt(socketFd, &tempInt);
    record = getBookInfo(bookFd, tempInt);
    if (tempInt <= 0)
    {
        iprintf(socketFd, "0은 사용할 수 없습니다.\n");
        return;
    }
    if (record.key != NO_DATA && record.key != 0)
    {
        iprintf(socketFd, "존재하는 도서식별자입니다.\n");
        return;
    }
    record.key = tempInt;
    iprintf(socketFd, " 도서명 : ");
    iscanf(socketFd, temp);
    strcpy(record.name, temp);
    iprintf(socketFd, " 저자 : ");
    iscanf(socketFd, temp);
    strcpy(record.author, temp);
    iprintf(socketFd, " 출판년월일 : ");
    iscanf(socketFd, temp);
    strcpy(record.release, temp);
    iprintf(socketFd, " 가격: ");
    iscanfInt(socketFd, &tempInt);
    record.price = tempInt;
    iprintf(socketFd, " 추천리뷰: ");
    iscanf(socketFd, temp);
    strcpy(record.review, temp);
    strcpy(record.memberId, m.id);

    iprintf(socketFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);
    setBookInfo(socketFd, bookFd, record);
    upBookHeader(bookFd);
}
int compareId(const void *b1, const void *b2)
{
    return strcmp(((book *)b1)->memberId, ((book *)b2)->memberId);
}
void listupAllbookById(int socketFd, int bookFd)
{
    book record;
    int count = getBookHeader(bookFd);
    int index = 0;
    book *recordList = (book *)malloc(sizeof(record) * count);
    int i = 0;
    while (1)
    {
        record = getBookInfo(bookFd, i);
        if (record.key == NO_DATA)
        {
            break;
        }
        if (record.key != 0)
        {
            recordList[index] = record;
            index++;
        }
        i++;
    }
    iprintf(socketFd, "도서식별자 도서명 저자 출판년월일 가격 소유자회원id 추천리뷰\n");
    qsort(recordList, count, sizeof(record), compareId);
    for (int i = 0; i < count; i++)
    {
        record = recordList[i];
        iprintf(socketFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);
    }
    free(recordList);
}

int comparePrice(const void *b1, const void *b2)
{
    return ((book *)b1)->price > ((book *)b2)->price;
}

void listupAllbookByPrice(int socketFd, int bookFd, member m)
{
    book record;
    int count = 0;
    int index = 0;
    int i = 0;
    while (1)
    {
        record = getBookInfo(bookFd, i);
        if (record.key == NO_DATA)
        {
            break;
        }
        if (record.key != 0)
        {
            if (strcmp(m.id, record.memberId) == 0)
            {
                count++;
            }
        }
        i++;
    }
    book *recordList = (book *)malloc(sizeof(record) * count);
    i = 0;
    while (1)
    {
        record = getBookInfo(bookFd, i);
        if (record.key == NO_DATA)
        {
            break;
        }
        if (record.key != 0)
        {
            if (strcmp(m.id, record.memberId) == 0)
            {
                recordList[index] = record;
                index++;
            }
        }
        i++;
    }
    iprintf(socketFd, "도서식별자 도서명 저자 출판년월일 가격 소유자회원id 추천리뷰\n");
    qsort(recordList, count, sizeof(record), comparePrice);
    for (int i = 0; i < count; i++)
    {
        record = recordList[i];
        iprintf(socketFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);
    }
    free(recordList);
}

void updateBook(int socketFd, int bookFd, member m)
{
    int sel = 0;
    iprintf(socketFd, "변경할 책의 식별자를 입력하세요\n");
    iscanfInt(socketFd, &sel);
    book record;
    record = getBookInfo(bookFd, sel);
    if (record.key == NO_DATA)
    {
        iprintf(socketFd, "변경할 책이 존재하지 않습니다.\n");
        return;
    }
    if (m.admin != 1 && strcmp(record.memberId, m.id) != 0)
    {
        iprintf(socketFd, "변경할 책이 존재하지 않습니다.\n");
        return;
    }

    iprintf(socketFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);
    char temp[30];
    int tempInt;
    iprintf(socketFd, " 도서명 : ");
    iscanf(socketFd, temp);
    strcpy(record.name, temp);
    iprintf(socketFd, " 저자 : ");
    iscanf(socketFd, temp);
    strcpy(record.author, temp);
    iprintf(socketFd, " 출판년월일 : ");
    iscanf(socketFd, temp);
    strcpy(record.release, temp);
    iprintf(socketFd, " 가격: ");
    iscanfInt(socketFd, &tempInt);
    record.price = tempInt;
    iprintf(socketFd, " 추천리뷰: ");
    iscanf(socketFd, temp);
    strcpy(record.review, temp);
    setBookInfo(socketFd, bookFd, record);
}

void deleteBook(int socketFd, int bookFd, member m)
{
    int sel = 0;
    iprintf(socketFd, "삭제할 책의 식별자를 입력하세요\n");
    iscanfInt(socketFd, &sel);
    book record;
    record = getBookInfo(bookFd, sel);
    if (record.key == NO_DATA)
    {
        iprintf(socketFd, "삭제할 책이 존재하지 않습니다.\n");
        return;
    }
    if (m.admin != 1 || strcmp(record.memberId, m.id) != 0)
    {
        iprintf(socketFd, "삭제할 책이 존재하지 않습니다.\n");
        return;
    }

    struct flock lock;

    iprintf(socketFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(record);
    lock.l_start = record.key * sizeof(record) + sizeof(int);
    lock.l_type = F_RDLCK;

    fcntl(bookFd, F_SETLKW, &lock);
    lseek(bookFd, sizeof(int) + sizeof(record) * record.key, SEEK_SET);
    record.key = 0;
    write(bookFd, (char *)&record, sizeof(record));

    lock.l_type = F_UNLCK;
    fcntl(bookFd, F_SETLK, &lock);
    downBookHeader(bookFd);
}

void searchByName(int socketFd, int bookFd, member m)
{
    book record;
    char temp[30];
    iprintf(socketFd, "도서명을 입력하세요\n");
    iscanf(socketFd, temp);
    int i = 0;
    while (1)
    {
        record = getBookInfo(bookFd, i);
        if (record.key == NO_DATA)
        {
            break;
        }
        if (record.key != 0)
        {
            if (m.admin == 1 || strcmp(m.id, record.memberId) == 0)
            {
                if (strstr(record.name, temp))
                {
                    iprintf(socketFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);
                }
            }
        }
        i++;
    }
}

void searchByAuthor(int socketFd, int bookFd, member m)
{
    book record;
    char temp[30];
    iprintf(socketFd, "저자를 입력하세요\n");
    iscanf(socketFd, temp);
    int i = 0;
    while (1)
    {
        record = getBookInfo(bookFd, i);
        if (record.key == NO_DATA)
        {
            break;
        }
        if (record.key != 0)
        {
            if (m.admin == 1 || strcmp(m.id, record.memberId) == 0)
            {
                if (strstr(record.author, temp))
                {
                    iprintf(socketFd, "%d %s %s %s %d %s %s\n", record.key, record.name, record.author, record.release, record.price, record.memberId, record.review);
                }
            }
        }
        i++;
    }
}

void listupAllMember(int socketFd, int memberFd, member m)
{
    member record;
    iprintf(socketFd, "회원번호 id password 이름 휴대폰번호 이메일 생년월일 admin여부\n");
    int i = 0;
    while (1)
    {
        record = getMemberInfo(memberFd, i);
        if (record.key == NO_DATA)
        {
            break;
        }
        if (record.key != 0)
        {
            iprintf(socketFd, "%d %s %s %s %s %s %s %d\n", record.key, record.id, record.password, record.name, record.phoneNum, record.email, record.birth, record.admin);
        }
        i++;
    }
}

void updateMember(int socketFd, int memberFd)
{
    int sel = 0;
    iprintf(socketFd, "변경할 회원의 식별자를 입력하세요\n");
    iscanfInt(socketFd, &sel);
    member record;
    record = getMemberInfo(memberFd, sel);
    if (record.key == NO_DATA || record.key == 0)
    {
        iprintf(socketFd, "변경할 회원이 존재하지 않습니다.\n");
        return;
    }

    iprintf(socketFd, "%d %s %s %s %s %s %s %d\n", record.key, record.id, record.password, record.name, record.phoneNum, record.email, record.birth, record.admin);
    char temp[30];
    int tempInt;
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

    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(record);
    lock.l_start = record.key * sizeof(record) + sizeof(int);
    lock.l_type = F_RDLCK;
    fcntl(memberFd, F_SETLKW, &lock);
    lseek(memberFd, sizeof(int) + sizeof(record) * record.key, SEEK_SET);
    int check = write(memberFd, (char *)&record, sizeof(record));
    lock.l_type = F_UNLCK;
    fcntl(memberFd, F_SETLK, &lock);
}

void deleteMember(int socketFd, int memberFd, int bookFd)
{
    int sel = 0;
    iprintf(socketFd, "삭제할 회원의 식별자를 입력하세요\n");
    iscanfInt(socketFd, &sel);
    member record;
    record = getMemberInfo(memberFd, sel);
    if (record.key == NO_DATA)
    {
        iprintf(socketFd, "삭제할 회원이 존재하지 않습니다.\n");
        return;
    }

    int i = 0;
    book bookrecord;
    while (1)
    {
        bookrecord = getBookInfo(bookFd, i);
        if (bookrecord.key == NO_DATA)
        {
            break;
        }
        if (bookrecord.key != 0)
        {
            if (strcmp(record.id, bookrecord.memberId) == 0)
            {
                iprintf(socketFd, "보유중인 책이 존재하여 삭제할 수 없습니다.\n");
                return;
            }
        }
        i++;
    }

    iprintf(socketFd, "%d %s %s %s %s %s %s %d\n", record.key, record.id, record.password, record.name, record.phoneNum, record.email, record.birth, record.admin);

    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(record);
    lock.l_start = record.key * sizeof(record) + sizeof(int);
    lock.l_type = F_RDLCK;
    fcntl(memberFd, F_SETLKW, &lock);
    lseek(memberFd, sizeof(int) + sizeof(record) * record.key, SEEK_SET);
    record.key = 0;
    int check = write(memberFd, (char *)&record, sizeof(record));
    lock.l_type = F_UNLCK;
    fcntl(memberFd, F_SETLK, &lock);
}

void updateMyInfo(int socketFd, int memberFd, member m)
{
    member record = m;
    iprintf(socketFd, "%d %s %s %s %s %s %s %d\n", record.key, record.id, record.password, record.name, record.phoneNum, record.email, record.birth, record.admin);
    char temp[30];
    int tempInt;
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

    iprintf(socketFd, "%d %s %s %s %s %s %s %d\n", record.key, record.id, record.password, record.name, record.phoneNum, record.email, record.birth, record.admin);
    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(record);
    lock.l_start = record.key * sizeof(record) + sizeof(int);
    lock.l_type = F_RDLCK;
    fcntl(memberFd, F_SETLKW, &lock);
    lseek(memberFd, sizeof(int) + sizeof(record) * record.key, SEEK_SET);
    int check = write(memberFd, (char *)&record, sizeof(record));
    lock.l_type = F_UNLCK;
    fcntl(memberFd, F_SETLK, &lock);
}

void bookMenu(int socketFd, int bookFd, int memberFd, member login)
{
    int sel = 0;
    if (login.admin == 0)
    {
        while (1)
        {
            printBookMenu(socketFd);
            iscanfInt(socketFd, &sel);
            if (sel == 0)
                break;
            else if (sel == 1)
            {
                nowtime(socketFd);
                printf("%s님이 도서를 검색 했습니다.", login.id);
                listupAllbook(socketFd, bookFd, login);
            }
            else if (sel == 2)
            {
                nowtime(socketFd);
                printf("%s님이 도서를 가격순으로 검색 했습니다.", login.id);
                listupAllbookByPrice(socketFd, bookFd, login);
            }
            else if (sel == 3)
            {
                nowtime(socketFd);
                printf("%s님이 도서를 추가 했습니다.", login.id);
                addBookInfo(socketFd, bookFd, login);
            }
            else if (sel == 4)
            {
                nowtime(socketFd);
                printf("%s님이 도서정보를 갱신 했습니다.", login.id);
                updateBook(socketFd, bookFd, login);
            }
            else if (sel == 5)
            {
                nowtime(socketFd);
                printf("%s님이 도서를 삭제 했습니다.", login.id);
                deleteBook(socketFd, bookFd, login);
            }
            else if (sel == 6)
            {
                nowtime(socketFd);
                printf("%s님이 도서명으로 책을 검색 했습니다.", login.id);
                searchByName(socketFd, bookFd, login);
            }
            else if (sel == 7)
            {
                nowtime(socketFd);
                printf("%s님이 저자명으로 책을 검색 했습니다.", login.id);
                searchByAuthor(socketFd, bookFd, login);
            }
            else if (sel == 8)
            {
                nowtime(socketFd);
                printf("%s님이 개인정보를 변경 했습니다.", login.id);
                updateMyInfo(socketFd, bookFd, login);
            }
        }
    }
    else
    {
        while (1)
        {
            printBookMenu_admin(socketFd);
            iscanfInt(socketFd, &sel);
            if (sel == 0)
                break;
            else if (sel == 1)
            {
                nowtime(socketFd);
                printf("%s님이 도서를 검색 했습니다.\n", login.id);
                listupAllbook(socketFd, bookFd, login);
            }
            else if (sel == 2)
            {
                nowtime(socketFd);
                printf("%s님이 id로 검색 했습니다.\n", login.id);
                listupAllbookById(socketFd, bookFd);
            }
            else if (sel == 3)
            {
                nowtime(socketFd);
                printf("%s님이 도서를 추가 했습니다.\n", login.id);
                addBookInfo(socketFd, bookFd, login);
            }
            else if (sel == 4)
            {
                nowtime(socketFd);
                printf("%s님이 도서정보를 갱신 했습니다.\n", login.id);
                updateBook(socketFd, bookFd, login);
            }
            else if (sel == 5)
            {
                nowtime(socketFd);
                printf("%s님이 도서를 삭제 했습니다.\n", login.id);
                deleteBook(socketFd, bookFd, login);
            }
            else if (sel == 6)
            {
                nowtime(socketFd);
                printf("%s님이 도서명으로 검색 했습니다.\n", login.id);
                searchByName(socketFd, bookFd, login);
            }
            else if (sel == 7)
            {
                nowtime(socketFd);
                printf("%s님이 회원리스트를 조회 했습니다.\n", login.id);
                listupAllMember(socketFd, memberFd, login);
            }
            else if (sel == 8)
            {
                nowtime(socketFd);
                printf("%s님이 신규 회원정보를 삽입 했습니다.\n", login.id);
                addMemberInfo(socketFd, memberFd);
            }
            else if (sel == 9)
            {
                nowtime(socketFd);
                printf("%s님이 회원정보를 삭제 했습니다.\n", login.id);
                deleteMember(socketFd, memberFd, bookFd);
            }
            else if (sel == 10)
            {
                nowtime(socketFd);
                printf("%s님이 회원정보를 갱신 했습니다.\n", login.id);
                updateMember(socketFd, memberFd);
            }
            else if (sel == 11)
            {
                nowtime(socketFd);
                printf("%s님이 개인정보를 변경 했습니다.\n", login.id);
                updateMyInfo(socketFd, bookFd, login);
            }
        }
    }
}