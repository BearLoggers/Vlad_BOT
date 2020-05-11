// это клиент

#include <iostream>
#include <SFML/Network.hpp>
#include <string>

int main()
{
	setlocale(LC_ALL, "rus");
	short choice = 0;
	std::string string;

	sf::TcpSocket socket;
	sf::Socket::Status status = socket.connect("localhost", 812);

	if (status != sf::Socket::Done)
	{
		std::cout << "Невозможно подключиться к серверу. Код статуса: " << status << '\n';
		exit(1);
	}

	while (true)
	{
		std::cout << "Чего пожелаете?\n 1 - Поиск по VK.\n 2 - Проигрывание с сырой ссылки.\n";
		std::cin >> choice;
		sf::Packet packet;

		switch (choice)
		{
		case 1:
			std::cout << "Введите название композиции: ";
			std::cin.ignore();
			std::getline(std::cin, string);
			if (string.empty())
				continue;
			packet << choice << string;
			socket.send(packet);
			
			std::cout << "Ожидаю ответа от сервера... ";
			socket.receive(packet);

			packet >> string;
			std::cout << string <<'\n';

			if (string == "success")
			{
				packet >> string;
				std::cout << string;
			}
			
			break;


		case 2:
			std::cout << "Введите ссылку: ";
			std::cin >> string;
			if (string.empty())
				continue;
			packet << choice << string;
			socket.send(packet);

			break;

		default:
			std::cout << "Я не понимаю, чего вы хотите.\n";
			break;
		}

	}
}