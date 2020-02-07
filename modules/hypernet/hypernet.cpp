#include "hypernet.h"
#include <fcntl.h>
#define PORT 4242

void	HyperNetManager::sendPacket(int type, int subtype, std::string data)
{
	std::string	packet;

	while (_listener_status == 0)
	{
		std::cout << "waiting\n";
	}
	if (_listener_status <= 0)
	{
		std::cout << "cannot send packet, network down\n";
		return ;
	}
	packet += to_string(type);
	packet += "|";
	packet += to_string(subtype);
	packet += "|";
	packet += data;
	packet += "]"
	std::cout << "sending\n";
	send(_sock, packet.c_str(), packet.length(), 0);
	std::cout << "sent\n";
	
}

void	*HyperNetManager::listener(void *d)
{
	int sock = 0;
	struct sockaddr_in serv_addr;
	int	auth = 0;

	if (!d)
	{

	}
	_listener_status = 0;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(4242);
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		std::cout << "invalid addr\n";
		_listener_status = -1;
		return (NULL);
	}
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
	{
		std::cout << "connection failed\n";
		if (errno)
			printf("%s\n", strerror(errno));
		_listener_status = -1;
		return (NULL);
	}
	_sock = sock;
	_listener_status = 1;
	int stat = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
	for (;;)
	{
		if (!auth)
			handshake();
		if (_listener_status == -1)
			return (NULL);
		if (read(sock, _buffer, 1024) > 0)
			std::cout << "RECIEVED: " << _buffer << std::endl;
		if (!auth)
		{
			//register authentication packet
		}
	}
	return (NULL);
}

std::string	HyperNetManager::genKey()
{
	std::string ret;
	int	res = 0;

	for (int i = 0; i != 8; i++)
	{
		res = rand() % 3;
		if (!res)
			ret += (char)((rand() % 26) + 97);
		else if (res == 1)
			ret += (char)((rand() % 26) + 65);
		else
			ret += (char)(('0' + rand() % 10));
	}
	return (ret);
}

HyperNetManager::HyperNetManager() : _buffer { 0 }
{
	_listener_status = 0;
	_sock = 0;
	int	fd = 0;
	char	buffer[1024] = { 0 };

	srand(time(NULL));
	std::ifstream f("./key");
	if (f.good())
	{
		std::filebuf	*pbuf = f.rdbuf();
		std::size_t		size = pbuf->pubseekoff(0, f.end, f.in);
		pbuf->pubseekpos(0, f.in);
		pbuf->sgetn(buffer, size);
		_key = std::string(buffer, buffer + size);
		f.close();
	}
	else
	{
		FILE *pfile;
		pfile = fopen("./key", "w");
		_key = genKey();
		fputs(_key.c_str(), pfile);
		fclose(pfile);
	}
	_t = std::thread(&HyperNetManager::listener, this, this);
}

HyperNetManager::~HyperNetManager()
{
	_listener_status = -1;
	_t.join();
	std::cout << "HyperNetManager ended!\n";
}

void	HyperNetManager::sendData(String s)
{
	while (_listener_status == 0)
	{
		std::cout << "waiting\n";
	}
	if (_listener_status <= 0)
	{
		std::cout << "cannot send packet, network down\n";
		return ;
	}
	std::cout << "sending\n";
	send(_sock, s.utf8(), s.length(), 0);
	std::cout << "sent\n";
}

void	HyperNetManager::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("sendData", "s"), &HyperNetManager::sendData);
}

void	HyperNetManager::setListenerStatus(int n) { _listener_status = n; }
int		HyperNetManager::getListenerStatus() const { return (_listener_status); }
