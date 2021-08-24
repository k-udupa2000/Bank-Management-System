
#define PORT 8080 

#define START_MENU 0

#define USER_OPTIONS 3
#define ADMIN_OPTIONS 4

#define EXIT 0
#define SIGN_IN 1
#define SIGN_UP 2
#define SIGN_IN_ADMIN 3
#define SIGN_IN_USER 4
#define SIGN_IN_JOINT 5
#define SIGN_UP_ADMIN 6
#define SIGN_UP_USER 7
#define SIGN_UP_JOINT 8
#define ERROR_OPTION 9
#define DEPOSIT_MONEY 10
#define WITHDRAW_MONEY 11
#define CHECK_BALANCE 12
#define PASSWORD_CHANGE 13
#define VIEW_DETAILS 14
#define SIGN_UP_ATTEMPT 15
#define SIGN_IN_ATTEMPT 16
#define ADD_USER 19
#define DELETE_USER 20
#define CHANGE_USER_DETAILS 21
#define GET_USER_DETAILS 22
#define INVALID -1
#define MESSAGE_SIZE 200

struct Admin{
    char password[MESSAGE_SIZE];
    char username[MESSAGE_SIZE];
};

struct User {
    char password[MESSAGE_SIZE];
    char username[MESSAGE_SIZE];
    int balance;
};

struct JointUser{
    char username1[MESSAGE_SIZE];
    char username2[MESSAGE_SIZE];
    char password[MESSAGE_SIZE];
    int balance;
};
