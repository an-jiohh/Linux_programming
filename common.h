#pragma once

typedef struct book
{
    int key;
    char name[30];
    char author[30];
    char release[30];
    char review[150];
    int price;
    char memberId[30];
} book;

typedef struct member
{
    int key;
    char id[30];
    char password[30];
    char name[30];
    char phoneNum[30];
    char email[30];
    char birth[30];
    int admin;
} member;