#include <iostream>
#include <cstring>
#include <strstream>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cos_offline_tread_pool.h>
#include "addressbook.pb.h"
#define MAXBUF 256
#define MAXLEN 32

using namespace Storage;
using namespace std;

class worker
{
	public:
		void set_ip_and_port(char *ip, int port);
		void set_person(char *name, char *email, char *id);
		static void worker_thread(void *client);

        private:
		int sockfd_, len_;
		socklen_t destsize_, srcsize_;
		struct sockaddr_in dest_, src_;
		char buffer_[MAXBUF + 1];
		string ip_;
		unsigned port_;
		addressbook::Person person_; 
};

void worker::set_ip_and_port(char *ip, int port)
{
	ip_ = ip;
	port_ = port;
}

void worker::set_person(char *name, char *email, char *id)
{
	person_.set_name(name);
	person_.set_email(email);
	person_.set_id(atoi(id));
}

void worker::worker_thread(void *client)
{
	worker *myclient = (worker *) client;
	if ((myclient->sockfd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket");
		exit(errno);
	}
	std::cout<<"socket craeted"<<endl;
	bzero(&(myclient->dest_), sizeof(myclient->dest_));
	myclient->dest_.sin_family = AF_INET;			
	myclient->dest_.sin_port = htons(myclient->port_);
	if (inet_aton(myclient->ip_.c_str(), (struct in_addr*) &(myclient->dest_.sin_addr.s_addr)) == 0)			
	{
		perror(myclient->ip_.c_str());
		exit(errno);
	}
	if (connect(myclient->sockfd_, (struct sockaddr*)&(myclient->dest_), sizeof(myclient->dest_)) == -1)
	{
		perror("Connect ");
		exit(errno);
	}
	cout<<"server connected"<<endl;
	pid_t pid;
	char name[MAXLEN+1], email[MAXLEN+1], id[MAXLEN+1], personinfo[MAXBUF+1];
	string str;
	if (-1 == (pid = fork()))
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		while (1)
		{
			bzero(myclient->buffer_, MAXBUF + 1);
			myclient->len_ = recv(myclient->sockfd_, myclient->buffer_, MAXBUF, 0);
			if (myclient->len_ > 0)
			{
				myclient->destsize_ = sizeof(myclient->dest_);
				myclient->srcsize_ = sizeof(myclient->src_);
				getpeername(myclient->sockfd_, (struct sockaddr*)&(myclient->dest_), &(myclient->destsize_));
				getsockname(myclient->sockfd_, (struct sockaddr*)&(myclient->src_), &(myclient->srcsize_));
				cout<<"port "<<ntohs(myclient->src_.sin_port)<<" recv successful: "<<myclient->buffer_<<", "<<myclient->len_<<" byte"<<endl;
			}
			else if (myclient->len_ < 0)
			{
				perror("recv");
				break;
			}
			else
			{
				std::cout<<"the other one port close, i will quit"<<endl;
				break;
			}
		}
	}
	else
	{
		while (1)
		{
			bzero(myclient->buffer_, MAXBUF + 1);
			bzero(personinfo, MAXBUF + 1);
			bzero(name, MAXLEN + 1);
			bzero(email, MAXLEN + 1);
			bzero(id, MAXLEN + 1);
			getsockname(myclient->sockfd_, (struct sockaddr*)&(myclient->src_), &(myclient->srcsize_));
			getpeername(myclient->sockfd_, (struct sockaddr*)&(myclient->dest_), &(myclient->destsize_));
			printf("pls send message to send\n");
			fgets(personinfo, MAXBUF, stdin);
			istrstream strin(personinfo);
			strin>>name>>email>>id;
			myclient->set_person(name, email, id);
			cout<<"start serializing"<<endl;
			str=myclient->buffer_;	
			myclient->person_.SerializeToString((string *) &str);
			strcpy(myclient->buffer_, str.c_str());
			//cout<<"testbegin\n********************************"<<endl;
			//addressbook::Person person_test;
			//string str_test;
			//str_test=myclient->buffer_;
			//person_test.ParseFromString((string &) str_test);
			//std::cout<<"'name': "<<person_test.name()<<std::endl;	
                        //std::cout<<"'email': "<<person_test.email()<<std::endl;
                        //std::cout<<"'id': "<<person_test.id()<<std::endl;	
			//cout<<"testend\n********************************"<<endl;
			if (!strncasecmp(name, "quit", 4))
			{
				cout<<"port "<<ntohs(myclient->src_.sin_port)<<" will quit the connect!" <<endl;
				//send(myclient->sockfd_, myclient->buffer_, 0, 0);
				break;
			}
			std::cout<<"ip "<<inet_ntoa(myclient->src_.sin_addr)<<" port "<<ntohs(myclient->src_.sin_port)<<" input is <</*"<<personinfo<<endl;
			myclient->len_ = send(myclient->sockfd_, myclient->buffer_, MAXBUF, 0);
			if (myclient->len_ < 0)
			{
				perror("send");
				break;
			}
		}
	}
	close(myclient->sockfd_);
}

int main(int argc, char *argv[])
{
	worker *client1 = new worker;
//	worker *client2 = new worker;
	ThreadPool<2> pool;
	client1->set_ip_and_port(argv[1],atoi(argv[2]));
//	client2->set_ip_and_port(argv[3],atoi(argv[4]));
	pool.AddJob([&]{worker::worker_thread((void *)client1);});
//	pool.AddJob([&]{worker::worker_thread((void *)client2);});
	return 0;
}
