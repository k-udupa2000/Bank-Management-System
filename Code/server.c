#include <fcntl.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include"struct.h"
struct flock lock;




int signUpUser(int,char*,char*, int); 
int signInUser(int,char*,char*);

void *clientHandler(void *);

int deposit(int, char*,int);
int withdraw(char*,int, int);
int balance(char*, int);
int change_password(char*,char*, int);
char* getUserDetails(char*, int);
int modifyUserDetails(char*,char*,char*, int, int);
int del_user(char*, int);

int main(int argc, char const *argv[]) 
{ 
	int server_fd, new_socket; 
	struct sockaddr_in addr; 
	int opt = 1; 
	int addrlen = sizeof(addr); 
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd == 0)
    {
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 

	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = INADDR_ANY; 
	addr.sin_port = htons(PORT); 
	
	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr))<0)
    { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 5) < 0) { 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
    printf("Server ready.\n");
    while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen))<0)
        { 
            printf("Accept system call error.\n");
            exit(EXIT_FAILURE); 
        } 
        pthread_t thread_id;
        if(pthread_create( &thread_id, NULL,  clientHandler, (void*) &new_socket) < 0)
        {
            printf("Thread creation error\n");
            return -1;
        }
        printf("New client found. New thread created.\n");
    }
    printf("Exiting server application");
	return 0; 
} 

void *clientHandler(void *socket_desc)
{
	int sock = *(int*)socket_desc , option, deposit_amt, withdraw_amt, ret,balance_amt;
	char* username = malloc(MESSAGE_SIZE*sizeof(char));
	char* password = malloc(MESSAGE_SIZE*sizeof(char));
	while(1)
    {
		char* new_username = malloc(MESSAGE_SIZE*sizeof(char));
		char* return_message = malloc(MESSAGE_SIZE*sizeof(char));
		char* option_string = malloc(MESSAGE_SIZE*sizeof(char));
		char* amt_string = malloc(MESSAGE_SIZE*sizeof(char));
		// printf("sock = %d\n",sock);
		read(sock, option_string, sizeof(option_string)); 
		option = atoi(option_string);
        if(option == EXIT)
        {
            close(sock);
            return 0;
        }
        int user_type;
        if(option == SIGN_UP_ADMIN || option == SIGN_IN_ADMIN)
            user_type = 0;
        if(option == SIGN_UP_USER || option == SIGN_IN_USER)
            user_type = 1;
        if(option == SIGN_UP_JOINT || option == SIGN_IN_JOINT)
            user_type = 2;
		if (option == SIGN_UP_ADMIN || option == SIGN_UP_JOINT || option == SIGN_UP_USER)
        {
			read(sock, username, sizeof(username));
			read(sock, password, sizeof(password));
			int ret = signUpUser(option,username,password, sock);
			if(ret == -1) return_message = "User could not be added\n";
			else return_message = "User added successfully!\n";
		}
        else if(option == SIGN_IN_ADMIN || option == SIGN_IN_JOINT || option == SIGN_IN_USER)
        {
			read(sock, username, sizeof(username));
			read(sock, password, sizeof(password));
			ret = signInUser(option,username,password);
			if(ret == -1) return_message = "sign in failed\n";
			else return_message = "successfully signed in!\n";
		}
		else if (option == DEPOSIT_MONEY)
        {
			// read(sock,amt_string,sizeof(amt_string));
			// deposit_amt = atoi(amt_string);
			ret = deposit(sock, username, user_type);
			if (ret == 0) return_message = "amount deposited\n";
		}
		else if (option == WITHDRAW_MONEY) {
			read(sock,amt_string,sizeof(amt_string));
			withdraw_amt = atoi(amt_string);
			ret = withdraw(username,withdraw_amt, user_type);
			if (ret == -1) return_message = "unable to withdraw\n";
			else return_message = "withdrew successfully\n";
		}
		else if (option == CHECK_BALANCE)
        {
			balance_amt = balance(username, user_type);
			sprintf(return_message,"%d",balance_amt);
		}
		else if (option == PASSWORD_CHANGE)
        {
			read(sock, password, sizeof(password));
			ret = change_password(username, password, user_type);
			if (ret == -1)
            {
                return_message = "Password change has failed.\n";
            }
			else
            {
                return_message = "Password change successful.\n";
            }
		}
		else if (option == VIEW_DETAILS)
        {
			return_message = getUserDetails(username, user_type);
		}
		else if(option == DELETE_USER) 
        {
			char* username = malloc(MESSAGE_SIZE*sizeof(char));
			char* password = malloc(MESSAGE_SIZE*sizeof(char));
			read( sock , username, sizeof(username));
            char* del_type_str = malloc(MESSAGE_SIZE*sizeof(char));
            read(sock, del_type_str, sizeof(del_type_str));
            int del_type = atoi(del_type_str);
            printf("del type = %d\n", del_type);
			ret = del_user(username, del_type);
			printf("unlink returned %d\n",ret);
			if (ret == -1) return_message = "unable to delete user\n";
			else return_message = "user deleted successfully\n";
		}
		else if(option == CHANGE_USER_DETAILS) {
			char* username = malloc(MESSAGE_SIZE*sizeof(char));
			char* password = malloc(MESSAGE_SIZE*sizeof(char));
            char* change_type_str = malloc(MESSAGE_SIZE*sizeof(char));
            printf("Read start\n");
			read(sock , username, sizeof(username));
			read(sock , new_username, sizeof(new_username));
			read(sock , password, sizeof(password));
            read(sock, change_type_str, sizeof(change_type_str));
            int old_type = atoi(change_type_str);
			ret = modifyUserDetails(username, new_username,password, old_type, sock);
			if (ret == -1) return_message = "unable to change user\n";
			else return_message = "changed user successfully\n";
		}
		else if (option == GET_USER_DETAILS) {
			char* username = malloc(MESSAGE_SIZE*sizeof(char));
            char* change_type_str = malloc(MESSAGE_SIZE*sizeof(char));

			read( sock , username, sizeof(username));
            read(sock, change_type_str, sizeof(change_type_str));
            int old_type = atoi(change_type_str);
			printf("username = %s\n",username);
			return_message = getUserDetails(username, old_type);
		}
		else if (option == ADD_USER) 
        {
            char* type = malloc(MESSAGE_SIZE*sizeof(char));
			char* username = malloc(MESSAGE_SIZE*sizeof(char));
			char* password = malloc(MESSAGE_SIZE*sizeof(char));
			read( sock , type, sizeof(type));
			read( sock , username, sizeof(username));
			read( sock , password, sizeof(password));
			printf("type = %s username = %s pwd = %s\n",type,username,password);
			if(!strcmp(type,"1")) option = SIGN_UP_USER;
			else option = SIGN_UP_JOINT;
			ret = signUpUser(option,username,password, sock);
			if(ret == -1) return_message = "account could not be added\n";
			else return_message = "successfully added account!\n";
		}
		send(sock , return_message , MESSAGE_SIZE * sizeof(char), 0); 
	}
    return 0;
} 



int signUpUser(int option, char* username, char* password, int sock){
    
    char filename[MESSAGE_SIZE];
    char* secondaryName = malloc(MESSAGE_SIZE*sizeof(char)); // useful for joint users.
    if(option == SIGN_UP_USER || option == ADD_USER)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(option == SIGN_UP_ADMIN)
    {
        char baseName[MESSAGE_SIZE] = "./Admin/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(option == SIGN_UP_JOINT)
    {
		
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }

    // strcpy(filename,username);
    char extension[5] = ".txt";
    int fd;
    strncat(filename,extension,sizeof(extension));
    printf("File = %s\n", filename);
    fd = open(filename,O_WRONLY);
    if(fd != -1)
    {
        return -1;
    }
    else
    {
        close(fd);
    }
    fd = open(filename,O_WRONLY | O_CREAT,0644);
    if(fd == -1)
    {
        printf("Sign up error\n");
        return -1;
    }
    if(option == SIGN_UP_USER || ADD_USER)
    {
        struct User user;
        strcpy(user.username,username);
        strcpy(user.password,password);
        user.balance = 0;
        write(fd,&user,sizeof(struct User));
        printf("initial balance = %d\n", user.balance);
    }
    else if(option == SIGN_UP_ADMIN)
    {
        struct Admin admin;
        strcpy(admin.username, username);
        strcpy(admin.password, password);
        write(fd,&admin,sizeof(struct Admin));
    }
    else if(option == SIGN_UP_JOINT)
    {
        read(sock, secondaryName, sizeof(secondaryName));
        struct JointUser jointuser;
        strcpy(jointuser.username1, username);
        strcpy(jointuser.username2, secondaryName);
        strcpy(jointuser.password, password);
        jointuser.balance = 0;
        write(fd,&jointuser,sizeof(struct JointUser));
    }
    close(fd);
    return 0;
}

int signInUser(int option, char* username, char* password)
{
    // static struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0; // Entire length of the file.
    lock.l_pid = getpid();


    char filename[MESSAGE_SIZE];
    if(option == SIGN_IN_USER)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
        // strncat
    }
    else if(option == SIGN_IN_ADMIN)
    {
        char baseName[MESSAGE_SIZE] = "./Admin/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(option == SIGN_IN_JOINT)
    {
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }

    char extension[5] = ".txt";
    int fd;
    strncat(filename,extension,sizeof(extension));
    printf("File = %s\n", filename);
    fd = open(filename,O_RDWR,0644);
    if(fd == -1){
        perror("signin"); return -1;
    }
    struct User u;
    int fd_lock = fcntl(fd, F_SETLKW, &lock);
    if(fd_lock == -1)
    {
        perror("fcntl");
        return -1;
    }
    // CS Starts here.
    lseek(fd,0,SEEK_SET);
    // printf("Beginning of CS. fd = %d, fd lock = %d\n", fd, fd_lock);
    read(fd,&u,sizeof(struct User));
    if((strcmp(u.password,password)!=0)) return -1;
    // end of critical 
    // getchar();
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    return 0;
}

int deposit(int sock, char* username, int user_type){
    // static struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0; // sizeof(struct User);
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct User);
    lock.l_pid = getpid();

    char filename[MESSAGE_SIZE];
    if(user_type == 0)
    {
        char baseName[MESSAGE_SIZE] = "./Admin/";
        strcat(baseName,username);
        strcpy(filename, baseName);
        // strncat
    }
    else if(user_type == 1)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(user_type == 2)
    {
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    // strcpy(filename,username);
    char extension[5] = ".txt";
    int fd;
    strncat(filename,extension,sizeof(extension));
    printf("File = %s\n", filename);
    fd = open(filename,O_RDWR,0644);
    if(fd == -1)
    {
        perror("signin");
        return -1;
    }
    struct User user;
    int fd_lock = fcntl(fd, F_SETLKW, &lock);
    printf("fd lock = %d\n", fd_lock);
    char* amt_string = malloc(MESSAGE_SIZE*sizeof(char));
    read(sock,amt_string,sizeof(amt_string));
    int amt = atoi(amt_string);
    if(fd_lock == -1) {perror("fcntl") ; return -1;}
    // CS Starts here.
    printf("Inside CS\n");
    if(read(fd,&user,sizeof(struct User))==-1) {perror("read"); return -1;}
    printf("user money initial = %d\n", user.balance);
    user.balance = user.balance + amt;
    printf("user money later = %d\n", user.balance);
    lseek(fd, 0, SEEK_SET);
    if(write(fd,&user,sizeof(struct User))==-1) {perror("write"); return -1;}
    // CS ends here.
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    return 0;
}

int withdraw(char* username, int amt, int user_type){
    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = sizeof(struct User);
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct User);
    lock.l_pid = getpid();

    char filename[MESSAGE_SIZE];
    if(user_type == 0)
    {
        char baseName[MESSAGE_SIZE] = "./Admin/";
        strcat(baseName,username);
        strcpy(filename, baseName);
        // strncat
    }
    else if(user_type == 1)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(user_type == 2)
    {
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    char extension[5] = ".txt";
    int fd;
    strncat(filename,extension,sizeof(extension));
    fd = open(filename,O_RDWR,0644);
    if(fd == -1){
        perror("signin"); return -1;
    }
    struct User user;
    if(fcntl(fd, F_SETLKW, &lock)==-1) {perror("fcntl") ; return -1;}
    // CS Starts here.
    // lseek(fd,sizeof(struct user),SEEK_SET);
    if(read(fd,&user,sizeof(struct User)) == -1) {perror("read"); return -1;}
    printf("Initial balance = %d\n",user.balance);
    if(user.balance - amt < 0) return -1;
    user.balance -= amt;
    printf("Balance later = %d\n", user.balance);
    // lseek(fd,sizeof(struct User),SEEK_SET);
    lseek(fd, 0, SEEK_SET);
    if(write(fd,&user,sizeof(struct User))==-1) {perror("write"); return -1;}
    // CS ends here.
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    return 0;
}

int balance(char* username, int user_type){
    static struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = sizeof(struct User);
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct User);
    lock.l_pid = getpid();

    char filename[MESSAGE_SIZE];
    if(user_type == 0)
    {
        char baseName[MESSAGE_SIZE] = "./Admin/";
        strcat(baseName,username);
        strcpy(filename, baseName);
        // strncat
    }
    else if(user_type == 1)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(user_type == 2)
    {
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    char extension[5] = ".txt";
    int fd;
    strncat(filename,extension,sizeof(extension));
    fd = open(filename,O_RDONLY,0644);
    if(fd == -1){
        perror("signin"); return -1;
    }
    struct User user;
    if(fcntl(fd, F_SETLKW, &lock)==-1) {perror("fcntl") ; return -1;}
    // CS Starts here.
    // lseek(fd,sizeof(struct User),SEEK_SET);
    if(read(fd,&user,sizeof(struct User))==-1) perror("read");
    // Send back to client.
    printf("Balance = %d\n", user.balance);
    // CS ends here.
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    return user.balance;
}

int change_password(char* username, char* pwd, int user_type){
    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct User);
    lock.l_pid = getpid();

    char filename[MESSAGE_SIZE];
    if(user_type == 0)
    {
        char baseName[MESSAGE_SIZE] = "./Admin/";
        strcat(baseName,username);
        strcpy(filename, baseName);
        // strncat
    }
    else if(user_type == 1)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(user_type == 2)
    {
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    char extension[5] = ".txt";
    int fd;
    strncat(filename,extension,sizeof(extension));
    fd = open(filename,O_RDWR,0644);
    if(fd == -1){
        perror("change pwd"); return -1;
    }
    struct User user;
    lseek(fd,0,SEEK_SET);
    if(fcntl(fd, F_SETLKW, &lock)==-1) {perror("fcntl") ; return -1;}
    // CS Starts here.
    if(read(fd,&user,sizeof(struct User))==-1) { perror("read"); return -1; }
    printf("Old pwd : %s\n", user.password);
    strcpy(user.password,pwd);
    lseek(fd,0,SEEK_SET);
    if(write(fd,&user,sizeof(struct User))==-1) { perror("write"); return -1; }
    printf("New pwd : %s\n", user.password);
    // CS ends here.
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    return 0;
}

char* getUserDetails(char* username, int user_type)
{
    static struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    lock.l_pid = getpid();

    char filename[MESSAGE_SIZE];
    if(user_type == 0)
    {
        char baseName[MESSAGE_SIZE] = "./Admin/";
        strcat(baseName,username);
        strcpy(filename, baseName);
        // strncat
    }
    else if(user_type == 1)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(user_type == 2)
    {
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    char extension[5] = ".txt";
    int fd;
    strncat(filename,extension,sizeof(extension));
    fd = open(filename,O_RDWR,0644);
    if(fd == -1)
    {
        printf("Open error\n");
        return "user does not exist\n";
    }
    printf("file = %s\n", filename);
    struct User user;
    if(fcntl(fd, F_SETLKW, &lock)==-1) {perror("fcntl"); return "sorry, section is locked\n";}
    // CS Starts here.
    // lseek(fd,0,SEEK_SET);
    if(read(fd,&user,sizeof(struct User))==-1)
        {perror("read"); return "unable to read file\n";}
    // CS ends here.
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    char* return_string = (char*)malloc(MESSAGE_SIZE * sizeof(char));
    // printf("user = %s\n", user.username);
    sprintf(return_string,"username : %s \npassword : %s\nbalance : %d\n", user.username, user.password, user.balance);
    return return_string;
}


int del_user(char* username, int del_type){
    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    lock.l_pid = getpid();

    char filename[MESSAGE_SIZE];
    

    if(del_type == 1)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    else if(del_type == 2)
    {
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
    }
    char extension[5] = ".txt";
    strncat(filename,extension,sizeof(extension));
    printf("File to be deleted = %s\n", filename);
    int fd = open(filename,O_RDWR,0644);
    if(fd == -1){ perror("open");}
    if(fcntl(fd, F_SETLKW, &lock)==-1) {perror("fcntl");}
    return unlink(filename);
}

int modifyUserDetails(char* username, char* new_username, char* password, int old_type, int sock) 
{
    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    lock.l_pid = getpid();
    char filename[MESSAGE_SIZE];
    int new_creation;
    if(old_type == 1)
    {
        char baseName[MESSAGE_SIZE] = "./Users/";
        strcat(baseName,username);
        strcpy(filename, baseName);
        new_creation = SIGN_UP_USER;
    }
    else if(old_type == 2)
    {
        char baseName[MESSAGE_SIZE] = "./JointUsers/";
        strcat(baseName,username);
        strcpy(filename, baseName);
        new_creation = SIGN_UP_JOINT;
    }
    char extension[5] = ".txt";
    int fd;
    strncat(filename,extension,sizeof(extension));
    printf("file = %s\n", filename);
    fd = open(filename,O_RDWR,0644);
    if(fd == -1){
        perror("mod user"); return -1;
    }
    struct User u;
    if(fcntl(fd, F_SETLKW, &lock)==-1) {perror("fcntl");}
    // CS Starts here.
    lseek(fd,0,SEEK_SET);
    if(read(fd,&u,sizeof(struct User))==-1) { perror("read"); return -1; }
    del_user(username, old_type);
    strcpy(u.username,new_username);
    signUpUser(new_creation,new_username,password, sock);
    // CS ends here.
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    return 0;
}