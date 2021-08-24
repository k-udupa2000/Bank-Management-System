#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h>
#include"struct.h"


int initialOptions();
int userChoice();
int adminChoice();
int printOptions(int);

void server_interface(int);
int authorizeUser(int,int);
void user_interface(int);
void admin_interface();

int main(int argc, char const *argv[]) 
{ 
	int sock = 0; 
	struct sockaddr_in serv_addr; 
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 
	server_interface(sock);
	close(sock);
	return 0;
} 

void server_interface(int sock){
	int option = initialOptions();
	while(option == ERROR_OPTION)
	{
		printf("Enter valid option\n");
		option = initialOptions();
	}
	if(option == EXIT)
	{
		char* exit_str = malloc(MESSAGE_SIZE*sizeof(char));
		sprintf(exit_str,"%d",EXIT);
		send(sock, exit_str, sizeof(exit_str), 0);
	}
	else
	{

		char* option_string = malloc(10*sizeof(char));
		char* return_message = malloc(MESSAGE_SIZE*sizeof(char));
		int access; 
			if(option == SIGN_IN_ADMIN)
			{
				access = authorizeUser(sock,SIGN_IN_ADMIN);
				if(access == -1)
				{
					printf("User details incorrect\n");
					// exit(1);
				}
				while(access != -1)
				{
					admin_interface(sock);
				}
			}
			if(option == SIGN_IN_USER)
			{
				access = authorizeUser(sock,SIGN_IN_USER);
				if(access == -1)
				{
					printf("User details incorrect\n");
					exit(1);
				}
				while(1)
				{
					user_interface(sock);
				}
			}
			if(option == SIGN_IN_JOINT)
			{
				access = authorizeUser(sock,SIGN_IN_JOINT);
				while(access != -1)
				{
					user_interface(sock);
				}
			}
			if(option == SIGN_UP_ADMIN)
			{
				access = authorizeUser(sock,SIGN_UP_ADMIN);
				while(access != -1)
				{
					admin_interface(sock);
				}
			}
			if(option == SIGN_UP_USER)
			{
				access = authorizeUser(sock, SIGN_UP_USER);
				while(access != -1)
				{
					user_interface(sock);
				}
			}
			if(option == SIGN_UP_JOINT)
			{
				access = authorizeUser(sock, SIGN_UP_JOINT);
				printf("Enter secondary name\n");
				char* secondaryUser = malloc(MESSAGE_SIZE*sizeof(char));
				scanf("%s",secondaryUser);
				send(sock, option_string, sizeof(secondaryUser), 0); 
				while(access != -1)
				{
					user_interface(sock);
				}
			}
	}
}

int authorizeUser(int sock, int option)
{
	char* username = malloc(MESSAGE_SIZE*sizeof(char));
	char* password = malloc(MESSAGE_SIZE*sizeof(char));
	char* return_message = malloc(MESSAGE_SIZE*sizeof(char));
	char* option_string = malloc(10*sizeof(char));
	int ret_option =  option;
	printf("Enter username : ");
	scanf("%s",username);
	printf("Enter password : ");
	scanf("%s",password);
	sprintf(option_string,"%d",ret_option);
	send(sock , option_string , sizeof(option_string), 0); 
	send(sock , username , sizeof(username) , 0 ); 
	send(sock , password , sizeof(password) , 0 ); 
	read( sock , return_message, MESSAGE_SIZE * sizeof(char)); 
	// printf("Return message = %s\n",return_message); 
	if(!strcmp(return_message,"sign in failed\n")) return -1;
	return ret_option;
}

void user_interface(int sock)
{
	// Display the initial options to users.
	int transactType = userChoice();
	// // if(logType == )
	int option,deposit_amt,withdraw_amt;
	char* option_string = malloc(10*sizeof(char));
	char* return_message = malloc(MESSAGE_SIZE*sizeof(char));
	char* amt_string = malloc(MESSAGE_SIZE*sizeof(char));
	char* password = malloc(MESSAGE_SIZE*sizeof(char));
	sprintf(option_string,"%d",transactType);
	send(sock , option_string , sizeof(option_string) , 0 ); 
	if(transactType == CHECK_BALANCE)
	{
		// There is no need of additional code here as the signal required is sent already.
		printf("The balance is,\n");
	}
	if(transactType == DEPOSIT_MONEY)
	{
		int deposit_amt;
		printf("Enter deposit amount\n");
		scanf("%d", &deposit_amt);
		sprintf(amt_string,"%d",deposit_amt);
		send(sock, amt_string, sizeof(amt_string), 0);
	}
	if(transactType == WITHDRAW_MONEY)
	{
		int withdraw_amt;
		printf("Enter amount to be withdrawn : ");
		scanf("%d", &withdraw_amt);
		sprintf(amt_string,"%d",withdraw_amt);
		send(sock, amt_string, sizeof(amt_string), 0);
	}
	if(transactType == PASSWORD_CHANGE)
	{
		printf("Enter the new password\n");
		scanf("%s", password);
		send(sock, password, sizeof(password), 0);
	}
	read(sock, return_message, MESSAGE_SIZE * sizeof(char)); 
	if(transactType == EXIT) exit(0);
	printf("Message from server = %s\n",return_message); 
}

void admin_interface(int sock){
	// int option = printOptions(ADMIN_OPTIONS), type;
	int transactType = adminChoice();
	if(transactType == EXIT)
		exit(0);

	char* option_string = malloc(10*sizeof(char));
	char* username = malloc(MESSAGE_SIZE*sizeof(char));
	char* new_username = malloc(MESSAGE_SIZE*sizeof(char));
	char* password = malloc(MESSAGE_SIZE*sizeof(char));
	char* return_message = malloc(MESSAGE_SIZE*sizeof(char));
	sprintf(option_string,"%d",transactType);
	send(sock , option_string , sizeof(option_string) , 0 ); 
	if(transactType == DELETE_USER)
	{
		printf("Enter username to be deleted : ");
		scanf("%s",username);
		send(sock , username , sizeof(username), 0); 
		printf("Do you want to delete a Normal user : 1 or Joint user : 2\n");
		int del_type;
		scanf("%d", &del_type);
		char del_type_str[MESSAGE_SIZE];
		sprintf(del_type_str, "%d", del_type);
		send(sock, del_type_str, sizeof(del_type_str), 0);

	}
	if(transactType == ADD_USER)
	{
		int user_type;
		printf("Enter User Type\n");
		printf("1 : Normal\n");
		printf("2 : Joint\n");
		scanf("%d",&user_type);
		printf("Enter username : ");
		scanf("%s",username);
		printf("Enter password : ");
		scanf("%s",password);
		if(user_type == 1)
			send(sock , "1" , sizeof("1") , 0 ); 
		else if(user_type == 2)
			send(sock , "2" , sizeof("2") , 0 ); 
		else
		{
			printf("Wrong option");
			exit(1);
		}
		printf("username = %s\n",username);
		printf("password = %s\n",password);
		send(sock , username , sizeof(username), 0); 
		send(sock , password , sizeof(password), 0);
	}
	if(transactType == VIEW_DETAILS)
	{
		printf("Enter username for search: ");
		scanf("%s",username);
		send(sock , username , sizeof(username), 0); 
	}
	if(transactType == CHANGE_USER_DETAILS)
	{
		printf("Enter old username : " );
		scanf("%s",username);
		send(sock , username , sizeof(username) , 0); 
		printf("Enter new username : ");
		scanf("%s",new_username);
		send(sock , new_username , sizeof(new_username) , 0 ); 
		printf("Enter user's password : ");
		scanf("%s",password);
		send(sock , password , sizeof(password) , 0 ); 
		
		printf("Is the user normal : 1 or joint user : 2\n");
		int old_user_type;
		scanf("%d", &old_user_type);
		char old_user_type_str[MESSAGE_SIZE];
		sprintf(old_user_type_str, "%d", old_user_type);
		send(sock, old_user_type_str, sizeof(old_user_type_str), 0);
	}
	if(transactType == GET_USER_DETAILS)
	{
		printf("Enter username : " );
		scanf("%s",username);
		send(sock , username , sizeof(username) , 0); 		
		printf("Is the user normal : 1 or joint user : 2\n");
		int old_user_type;
		scanf("%d", &old_user_type);
		char old_user_type_str[MESSAGE_SIZE];
		sprintf(old_user_type_str, "%d", old_user_type);
		send(sock, old_user_type_str, sizeof(old_user_type_str), 0);
	}
	read( sock , return_message, MESSAGE_SIZE * sizeof(char)); 
	printf("Return message = %s\n",return_message);  		
}


int initialOptions()
{
    printf("Select one of the following numbers for respective actions.\n");
    printf("1 : Sign up (new user)\n");
    printf("2 : Sign in (old user)\n");
    printf("3 : Exit\n");
    int option;
    scanf("%d", &option);
    if(option == 1)
    {
        printf("Do you want to sign up as the,\n");
        printf("1 : Administrator\n 2 : Normal User\n 3 : Joint User\n");
        int signupOption;
        scanf("%d", &signupOption);
		if(signupOption == 1)
		{
        	// if(ADMIN_EXISTS) return ERROR_OPTION;
			return SIGN_UP_ADMIN;
		}
		else if(signupOption == 2)
		{
			return SIGN_UP_USER;
		}
		else if(signupOption == 3)
		{
			return SIGN_UP_JOINT;
		}
    }
    else if(option == 2)
    {
        printf("Do you want to sign in as the,\n");
        printf("1 : Administrator\n 2 : Normal User\n 3 : Joint User\n");
        int signInOptions;
        scanf("%d", &signInOptions);
        switch (signInOptions)
        {
            case 1:
                return SIGN_IN_ADMIN;
            case 2:
                return SIGN_IN_USER;
            case 3:
                return SIGN_IN_JOINT;
            default:
                return ERROR_OPTION;
        }
    }
    else if(option == 3)
        return EXIT;
    return ERROR_OPTION;
}

int userChoice()
{
	int userOption;
	printf("Select among the following options\n");
	printf("1 : Deposit\n");
	printf("2 : Withdraw\n");
	printf("3 : Check Balance\n");
	printf("4 : Change Password\n");
	printf("5 : View Details\n");
	printf("6 : Exit\n");
	scanf("%d", &userOption);
	if(userOption == 1) return DEPOSIT_MONEY;
	if(userOption == 2) return WITHDRAW_MONEY;
	if(userOption == 3) return CHECK_BALANCE;
	if(userOption == 4) return PASSWORD_CHANGE;
	if(userOption == 5) return VIEW_DETAILS;
	if(userOption == 6) return EXIT;
	return ERROR_OPTION;
}

int adminChoice()
{
	printf("Select one among the following admin choices\n");
	int adminOptions;
	printf("1 : Add User\n");
	printf("2 : Delete User\n");
	printf("3 : Modify User\n");
	printf("4 : Search for Account Details\n");
	printf("5 : Exit\n") ;
	scanf("%d",&adminOptions);
	if(adminOptions == 1) return ADD_USER;
	if(adminOptions == 2) return DELETE_USER;
	if(adminOptions == 3) return CHANGE_USER_DETAILS;
	if(adminOptions == 4) return GET_USER_DETAILS;
	if(adminOptions == 5) return EXIT;
}
