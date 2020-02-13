#include "hypernet.h"
#include <fcntl.h>
#define PORT 4242

std::string	HyperNetManager::structurePacket(int type, int subtype, std::string data)
{
	std::string	ret;

	ret += std::to_string(type);
	ret += "|";
	ret += std::to_string(subtype);
	ret += "|";
	ret += data;
	ret += "]";
	return (ret);
}

unsigned	HyperNetManager::timeSinceLastTick(void) {
	return unsigned((clock() - _ticks) * 1000.0/CLOCKS_PER_SEC);
}

void	HyperNetManager::ticker(void)
{
	if (timeSinceLastTick() > 250)
	{
		_ticks = clock();
		send(_sock, _packetStack.c_str(), _packetStack.length(), 0);
		_packetStack.clear();
	}
}

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
	packet = structurePacket(type, subtype, data);
	std::cout << "sending" << packet << std::endl;
	_packetStack += packet;
	send(_sock, packet.c_str(), packet.length(), 0);
	//std::cout << "sent\n";
}

std::string	HyperNetManager::processPacket(std::string packet)
{
	std::istringstream f(packet);
	std::vector<std::string> substrs;
	std::string s;

	std::cout << "processing " << packet << std::endl;
	while (getline(f, s, '|'))
		substrs.push_back(s);
	if (std::stoi(substrs[0]) == SYSTEM)
	{
		if (substrs.size() == 7 && std::stoi(substrs[1]) == REGISTER_UNIT)
		{
			try
			{
				Variant	id(String(substrs[2].c_str()));
				Variant	hp(String(substrs[3].c_str()));
				Variant	x(String(substrs[4].c_str()));
				Variant	y(String(substrs[5].c_str()));
				Variant	type(String(substrs[6].c_str()));
				Variant	*args[5] = {&id, &hp, &x, &y, &type};
				Variant::CallError err;
				while (_bindings.count("register_unit") == 0);
				_bindings["register_unit"]->call_func((const Variant**)args, 5, err);
			}
			catch (std::exception &e)
			{
				std::cout << "request failed\n";
			}
		}
		else if (substrs.size() == 5 && std::stoi(substrs[1]) == UPDATE_UNIT)
		{
			std::cout << "updating unit " << substrs[2] << std::endl;
			Variant	id(String(substrs[2].c_str()));
			Variant	x(String(substrs[3].c_str()));
			Variant	y(String(substrs[4].c_str()));
			Variant	*args[3] = {&id, &x, &y};
			Variant::CallError err;
			while (_bindings.count("update_unit") == 0);
			_bindings["update_unit"]->call_func((const Variant**)args, 3, err);
		}
		else
			std::cout << "invalid request" << std::stoi(substrs[1]) << std::endl;
	}
	else
		std::cout << "user request\n";
	return (std::string(""));
}

int		HyperNetManager::moveUnit(std::string id, int dir)
{
	std::cout << "moveUnit called!\n";
	std::string	out(id);
	out += "|";
	out += std::to_string(dir);
	sendPacket(USER, MOVE, out);
	return (1);
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
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
	for (;;)
	{
		if (_luaStatus == -1)
		{
			_luaStatus = 0;
			_luaThread.join();
		}
		if (!auth)
			sendPacket(SYSTEM, HANDSHAKE, _key);
		if (_listener_status == -1)
			return (NULL);
		if (read(sock, _buffer, 1024) > 0)
		{
			std::string	in(_buffer, strlen(_buffer));
			std::istringstream f(in);
			std::string s;
			std::cout << "IN: " << _buffer << std::endl;
			while (getline(f, s, ']'))
				processPacket(s);
		}
		if (!auth)
		{
			std::cout << "auth successful\n";
			auth++;
			//register authentication packet
		}
		for (int i = 0; i != 1024; i++)
			_buffer[i] = 0;
		ticker();
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

void	*HyperNetManager::threadScript(void *data)
{
	char	*c = (char*)data;

	std::cout << c << std::endl;
	if (luaL_dofile(_L, c))
	{
		std::cout << "lua error " << lua_tostring(_L, -1) << std::endl;
		lua_pop(_L, 1);
	}
	_luaStatus = -1;
	delete[] c;
	return (NULL);
}

void HyperNetManager::runScript(String filepath)
{
	std::wstring ws = filepath.c_str();
	std::string s(ws.begin(), ws.end());
	char	*c = (char*)s.c_str();
	char	*f = new char[strlen(c) + 1];
	strcpy(f, c);
	f[strlen(c)] = 0;
	_luaStatus = 1;

	std::cout << c << std::endl;
	lua_settop(_L, 0);
	_luaThread = std::thread(&HyperNetManager::threadScript, this, f);
	lua_gettop(_L);
}

HyperNetManager::HyperNetManager() : _buffer { 0 }
{
	_listener_status = 0;
	_sock = 0;
	char	buffer[1024] = { 0 };
	_luaStatus = 0;
	_ticks = clock();

	_L = luaL_newstate();
	luaL_openlibs(_L);
	luabridge::getGlobalNamespace(_L)
		.beginClass<HyperNetManager>("HyperNetManager")
			.addFunction("moveUnit", &HyperNetManager::moveUnit)
		.endClass();
	luabridge::push(_L, this);
	lua_setglobal(_L, "manager");
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
	if (_luaStatus == 1)
		std::cout << "waiting for lua thread to close...\n";
	while (_luaStatus == 1);
	if (_luaStatus == -1)
		_luaThread.join();
	lua_close(_L);
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
	ClassDB::bind_method(D_METHOD("runScript", "filepath"), &HyperNetManager::runScript);
	ClassDB::bind_method(D_METHOD("registerCallback", "s", "func"), &HyperNetManager::registerCallback);
}

void	HyperNetManager::registerCallback(String s, Ref<FuncRef> r)
{
	std::cout << "registering " << s.utf8() << std::endl;
	_bindings.insert(std::make_pair(std::string(s.utf8()), r));
}

void	HyperNetManager::setListenerStatus(int n) { _listener_status = n; }
int		HyperNetManager::getListenerStatus() const { return (_listener_status); }
