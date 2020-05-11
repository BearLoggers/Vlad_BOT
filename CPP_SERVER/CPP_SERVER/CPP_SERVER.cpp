//это сервер
#include <iostream>
#include <SFML/Network.hpp>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>

// Для функции exec
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe)
	{
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) 
	{
        result += buffer.data();
    }
    return result;
}

int main()
{
	setlocale(LC_ALL, "rus"); 
	sf::TcpListener listener;
	sf::SocketSelector selector;

	std::ofstream file;

	std::vector<sf::TcpSocket*> sockets;

	if (listener.listen(812) != sf::Socket::Done)
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

	std::cout << "Сервер слушает 812\n";
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

							std::string data, temp, url, status;

							switch(choice)
							{
							// Поиск по VK
							case 1:
							{
								// FIXME: Это можно убить
								packet >> data;
								std::cout << "Клиент хочет искать по ВК: " << data << std::endl;

								//std::cout << "Внимание, сейчас будет флекс...\n";
								temp = "cd ../../KISSVK_PARSER && phantomjs.exe loadmusicSilent.js \"" + data + "\"";

								bool isFinished = false;
								std::thread getSongInfoThread([&temp, &isFinished]()
								{
									temp = exec(temp.c_str());
									isFinished = true;
								});
								getSongInfoThread.detach();

								for (int i = 0; i < 100; i++) {
									if (isFinished) break;
									std::this_thread::sleep_for(std::chrono::milliseconds(150));
								}
								if (!isFinished)
								{
									std::cout << "Таймаут, убиваем поток c KISSVK\n";
									getSongInfoThread.~thread();
									status = "timeout";
									packet.clear();
									packet << status;
									sockets[i]->send(packet);
									continue;
								}

								// Сначала URL, потом после переноса строки автор и название
								url = temp.substr(0, temp.find('\n'));

								if (url.substr(0, 4) != "http")
								{
									std::cout << "Не удалось получить ссылку\n";
									status = "failed";
									packet.clear();
									packet << status;
									sockets[i]->send(packet);
									//TODO: Проиграть БАААШЕР / Отправить клиенту обратно
								}
								else
								{
									file.open("../../JS_BOT/link.txt");
									file << url;
									file.close();

									status = "success";
									packet.clear();
									packet << status << temp.substr(temp.find('\n') + 1);
									sockets[i]->send(packet);
								}

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