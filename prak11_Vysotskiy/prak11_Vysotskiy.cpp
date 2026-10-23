#include <windows.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <conio.h>

using namespace std;

struct Robot
{
    int id;
    int energy;
    int points;
    int rnddur = 0;
    HANDLE thread;
};

Robot robots[10];

int tests[100];
int testCount = 0;

DWORD WINAPI RobotThread(LPVOID mahabit)
{
    Robot* robot = (Robot*)mahabit;
    srand(GetTickCount() + robot->id * 777);
    for (int i = 0; i < testCount; i++)
    {
        int test = tests[i];
        int duration = 0;
        int energyCost = 0;
        int points = 0;
        

        if (test == 1)
        {
            duration = 3;
            energyCost = 10;
            points = 15;
        }
        else if (test == 2)
        {
            duration = 4;
            energyCost = 20;
            points = 30;
        }
        else if (test == 3)
        {   
            duration = rand() % 5 + 1;
        }
        else if (test == 4)
        {
            duration = 10;
            energyCost = 60;
            points = 55;
        }
        cout << endl << "Робот " << robot->id << ": тест " << test << endl;

        if (test == 3)
        {
            cout << endl << "Робот " << robot->id << " кайфует " << duration << " сек" << endl;

            Sleep(duration * 500);

            robot->energy += duration * 10;

            if (robot->energy > 100) robot->energy = 100;

            cout << endl << "Робот " << robot->id << ": энергия = " << robot->energy << endl;
        }
        else if (robot->energy < energyCost)
        {
            cout << "Робот " << robot->id << ": недостаточно энергии, тест пропущен" << endl;
        }
        else
        {
            cout << "Робот " << robot->id << " выполняет " << duration << " сек" << endl;

            Sleep(duration * 1000);
            
            robot->energy -= energyCost;
            
            robot->points += points;
            
            cout << endl << "Робот " << robot->id << ": энергия = " << robot->energy << ", очки = " << robot->points << endl;
        }

        if (i < testCount - 1)
        {
            int delay = rand() % 3 + 1;

            cout << "Робот " << robot->id << ": перерыв " << delay << " сек" << endl;

            Sleep(delay * 1000);
        }
    }

    cout << "Робот " << robot->id << " завершил работу" << endl;

    return 0;
}

int main()
{
    setlocale(LC_ALL, "Russian");
    srand(time(NULL));

    int robotCount;

    cout << "Количество роботов (3-10): ";
    cin >> robotCount;

    if (robotCount < 3 || robotCount > 10)
    {
        cout << "Неверное количество роботов" << endl;
        return 0;
    }

    ifstream file("arena.dat", ios::binary);

    if (file.is_open())
    {
        int test;

        while (file.read((char*)&test, sizeof(test)))
        {
            if (test >= 1 && test <= 4 && testCount < 100)
            {
                tests[testCount] = test;
                testCount++;
            }
        }

        file.close();
    }

    if (testCount == 0)
    {
        ofstream outFile("arena.dat", ios::binary);

        for (int i = 0; i < 10; i++)
        {
            tests[i] = rand() % 4 + 1;
            outFile.write((char*)&tests[i], sizeof(int));
        }

        outFile.close();
        testCount = 10;

        cout << "Создан файл arena.dat с тестами" << endl;
    }

    cout << "Всего тестов: " << testCount << endl;
    cout << "Команды управления:" << endl;
    cout << "p (номер) - приостановить" << endl;
    cout << "r (номер) - продолжить" << endl;
    cout << "s - приостановить всех" << endl;
    cout << "w - продолжить всех" << endl;
 
    for (int i = 0; i < robotCount; i++)
    {
        robots[i].id = i + 1;
        robots[i].energy = 60;
        robots[i].points = 0;
        robots[i].thread = NULL;

        robots[i].thread = CreateThread(NULL,0,RobotThread,&robots[i],0,NULL);

        if (robots[i].thread == NULL)
        {
            cout << "Не удалось создать поток робота " << i + 1 << endl;
            return 1;
        }
    }

    char commandLine[100];
    int lineLength = 0;
    bool finished = false;

    while (!finished)
    {
        DWORD result = WaitForMultipleObjects(robotCount,&robots[0].thread,TRUE,0);
        finished = true;

        for (int i = 0; i < robotCount; i++)
        {
            if (WaitForSingleObject(robots[i].thread, 0) != WAIT_OBJECT_0)
            {
                finished = false;
            }
        }

        if (finished)
            break;

        if (_kbhit())
        {
            char ch = _getch();

            if (ch == '\r')
            {
                char command = commandLine[0];
                int id = 0;
                int a = 2;

                while (commandLine[a] >= '0' && commandLine[a] <= '9')
                {
                    id = id * 10 + (commandLine[a] - '0');
                    a++;
                }
                if (command == 'p' || command == 'P')
                {
                    if (SuspendThread(robots[id - 1].thread) != (DWORD)-1)
                    {
                        cout << "Робот " << id << " приостановлен" << endl;

                    }
                }

                else if (command == 'r' || command == 'R')
                {
                    if (ResumeThread(robots[id - 1].thread) != (DWORD)-1)
                    {
                        cout << "Робот " << id << " продолжил " << endl;
                    }
                }
                else if (command == 's' || command == 'S')
                {
                    for (int i = 1; i < 11; i++)
                    {
                        SuspendThread(robots[i - 1].thread);
                    }
                    cout << " Все роботы приостановлены" << endl;
                }
                else if (command == 'w' || command == 'W')
                {
                    for (int i = 1; i < 11; i++)
                    {
                        ResumeThread(robots[i - 1].thread);
                    }
                    cout << " Роботы продолжают" <<  endl;
                }
            
                lineLength = 0;
            }
            else if (ch == '\b')
            {
                if (lineLength > 0)
                {
                    lineLength--;
                    cout << "\b \b";
                }
            }
            else if (lineLength < 98)
            {
                commandLine[lineLength] = ch;
                lineLength++;
                cout << ch;
            }
        }

        Sleep(50);
    }

    WaitForMultipleObjects(robotCount,&robots[0].thread,TRUE,INFINITE);

    for (int i = 0; i < robotCount; i++)
    {
        CloseHandle(robots[i].thread);
        robots[i].thread = NULL;
    }

    for (int i = 0; i < robotCount - 1; i++)
    {
        for (int j = 0; j < robotCount - i - 1; j++)
        {
            if (robots[j].points < robots[j + 1].points)
            {
                Robot temp = robots[j];
                robots[j] = robots[j + 1];
                robots[j + 1] = temp;
            }
        }
    }

    cout << "Итоги: " << endl;

    for (int i = 0; i < robotCount; i++)
    {
        cout << "Место: " << i + 1 << ", робот: "
            << robots[i].id << ", очки: " << robots[i].points
            << ", энергия: " << robots[i].energy << endl;
    }

    if (robotCount > 1 &&
        robots[0].points == robots[1].points)
    {
        cout << "Ничья" << endl;
    }
    else
    {
        cout << "Победил робот " << robots[0].id << endl;
    }
    return 0;
}