//это сервер
#include <iostream>
#include <SFML/Network.hpp>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>

#include <locale>
#include <codecvt>
#include <io.h>
#include <fcntl.h>

bool fileExists(const char* fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

int main()
{
	setlocale(LC_ALL, "rus"); 
	sf::TcpListener listener;
	sf::SocketSelector selector;

	std::ofstream file;

	std::vector<sf::TcpSocket*> sockets;
	
	int port;
	std::cout << "Введите порт: ";
	std::cin >> port;

	if (listener.listen(port) != sf::Socket::Done)
	{
		std::cout << "Err.";	
		exit(1);
	}

	// Создаем поток, в котором запускаем ноду
	std::thread nodeThread([]() 
	{
		system("cd ../../JS_BOT && node ./server.js");
		//system("node ./server.js");
	});

	nodeThread.detach();

	std::cout << "Сервер слушает " << port << '\n';
	selector.add(listener);

	while (true)
	{
		if (selector.wait())
		{
			std::cout << "сервер дождался" << std::endl;
			if (selector.isReady(listener))
			{
				sf::TcpSocket* socket = new sf::TcpSocket;

				if (listener.accept(*socket) != sf::Socket::Done)
				{
					// Ошибка при подключении
					std::cout << "Что-то пошло не так :(" << std::endl;
					//sockets.erase(socket);
				}
				else
				{
					// Подключение успешно, добавляем к прослушке
					std::cout << "Получил нового клиента" << std::endl;
					selector.add(*socket);
					sockets.push_back(socket);
				}
			}
			// Проходимся по добавленым сокетам и определяем, кто из них сработал
			else 
			{
				for (int i = 0; i < sockets.size(); i++)
				{
					if (selector.isReady(*sockets[i]))
					{
						// Принимаем входящий пакет от сокета
						sf::Packet packet;
						sf::Socket::Status status = sockets[i]->receive(packet);
						if (status == sf::Socket::Disconnected)
						{
							selector.remove(*sockets[i]);
							delete sockets[i];							
							sockets.erase(sockets.begin() + i);
							std::cout << "Клиент удачно отключился.\n";
							continue;
						}
						else if (status == sf::Socket::Done)
						{
							std::cout << "Принял немного данных: " << std::endl;
							// Сначала понимаем, что выбрал пользователь
							short choice;
							// FIXME: Это можно убить
							packet >> choice;

							std::cout << "Выбор: " << choice << std::endl;

							std::string data, temp, url;

							switch(choice)
							{
							// Поиск по VK
							case 1:
							{
								// FIXME: Это можно убить
								packet >> data;
								std::cout << "Клиент хочет искать по ВК: " << data << std::endl;

								file.open("../../JS_BOT/vkquery.txt");
								file << data;
								file.close();

								// Ждём результата от Node.JS
								// std::cout << "Я ждун...\n";
								while (!fileExists("vksearch.status"));
								// std::cout << "Больше нет...\n";
								std::ifstream status("vksearch.status");
								std::string string;
								std::getline(status, string, '\n');
								if (string == "success") {
									// Название и автор
									std::getline(status, string, '\n');

									packet.clear();
									packet << "success" << string;
									sockets[i]->send(packet);
								}
								else {
									packet.clear();
									packet << "failed" << string;
									sockets[i]->send(packet);
								}

								status.close();
								system("del vksearch.status");

								break;
							}

							// Произвольная ссылка
							case 2:
								// FIXME: Это можно убить
								packet >> data;
								std::cout << "Клиент хочет воспроизводить по ссылке: " << data << std::endl;
								file.open("../../JS_BOT/link.txt");
								file << data;
								file.close();
								break;

							default:
								std::cout << "Оу, да у вас модифицированный клиентб\n";
								break;
							}
						}
						else 
						{
							// Ошибка при приёме пакета
							std::cout << "Ошибка при приёме пакета :(\n";
						}
					}				
				}
			}
		}
		else
		{
			std::cout << "Ошибка во время ожидания (какая?).\n";
		}
	}
}