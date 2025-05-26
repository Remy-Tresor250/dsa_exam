#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <cfloat>
#include <climits>

using namespace std;

struct City
{
    int index;
    string name;

    City() : index(0), name("") {}
    City(int idx, string n) : index(idx), name(n) {}
};

struct Road
{
    int number;
    string roadName;
    double budget;
    int city1Index;
    int city2Index;

    Road() : number(0), roadName(""), budget(0.0), city1Index(0), city2Index(0) {}
    Road(int num, string name, double b, int c1, int c2)
        : number(num), roadName(name), budget(b), city1Index(c1), city2Index(c2) {}
};

class CitiesManagementSystem
{
private:
    vector<City> cities;
    vector<Road> roads;
    vector<vector<int>> adjacencyMatrix;
    vector<vector<double>> budgetMatrix;

    // Input validation helper functions
    int getValidInteger(const string &prompt, int minValue = INT_MIN, int maxValue = INT_MAX)
    {
        int value;
        string input;

        while (true)
        {
            cout << prompt;
            getline(cin, input);

            // Check if input is empty
            if (input.empty())
            {
                cout << "Error: Please enter a valid integer.\n";
                continue;
            }

            // Check if input contains only digits (and optional negative sign)
            bool isValid = true;
            size_t start = 0;
            if (input[0] == '-')
            {
                start = 1;
                if (input.length() == 1)
                {
                    isValid = false;
                }
            }

            for (size_t i = start; i < input.length(); i++)
            {
                if (!isdigit(input[i]))
                {
                    isValid = false;
                    break;
                }
            }

            if (!isValid)
            {
                cout << "Error: Please enter a valid integer.\n";
                continue;
            }

            try
            {
                value = stoi(input);
                if (value < minValue || value > maxValue)
                {
                    cout << "Error: Please enter a value between " << minValue << " and " << maxValue << ".\n";
                    continue;
                }
                break;
            }
            catch (const exception &)
            {
                cout << "Error: Please enter a valid integer.\n";
            }
        }

        return value;
    }

    double getValidDouble(const string &prompt, double minValue = -DBL_MAX, double maxValue = DBL_MAX)
    {
        double value;
        string input;

        while (true)
        {
            cout << prompt;
            getline(cin, input);

            // Check if input is empty
            if (input.empty())
            {
                cout << "Error: Please enter a valid number.\n";
                continue;
            }

            try
            {
                value = stod(input);
                if (value < minValue || value > maxValue)
                {
                    cout << "Error: Please enter a value between " << minValue << " and " << maxValue << ".\n";
                    continue;
                }
                break;
            }
            catch (const exception &)
            {
                cout << "Error: Please enter a valid number.\n";
            }
        }

        return value;
    }

    string getValidString(const string &prompt, bool allowEmpty = false)
    {
        string input;

        while (true)
        {
            cout << prompt;
            getline(cin, input);

            // Trim whitespace
            input.erase(0, input.find_first_not_of(" \t\n\r"));
            input.erase(input.find_last_not_of(" \t\n\r") + 1);

            if (!allowEmpty && input.empty())
            {
                cout << "Error: Please enter a non-empty string.\n";
                continue;
            }

            break;
        }

        return input;
    }

    int getValidMenuChoice(int minChoice, int maxChoice)
    {
        return getValidInteger("Enter your choice: ", minChoice, maxChoice);
    }

    void clearInputBuffer()
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

public:
    CitiesManagementSystem()
    {
        loadCitiesFromFile();
        loadRoadsFromFile();
        updateMatrices();
    }

    ~CitiesManagementSystem()
    {
        saveCitiesToFile();
        saveRoadsToFile();
    }

    void loadCitiesFromFile()
    {
        ifstream file("cities.txt");
        if (!file.is_open())
        {
            cout << "cities.txt not found. Starting with empty city list.\n";
            return;
        }

        string line;
        getline(file, line); // Skip header

        while (getline(file, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string indexStr, cityName;
            ss >> indexStr;
            getline(ss, cityName);

            // Remove leading spaces from city name
            cityName.erase(0, cityName.find_first_not_of(" \t"));

            if (!indexStr.empty() && !cityName.empty())
            {
                try
                {
                    cities.push_back(City(stoi(indexStr), cityName));
                }
                catch (const exception &)
                {
                    cout << "Warning: Invalid city data in file: " << line << "\n";
                }
            }
        }
        file.close();
    }

    void loadRoadsFromFile()
    {
        ifstream file("roads.txt");
        if (!file.is_open())
        {
            cout << "roads.txt not found. Starting with empty roads list.\n";
            return;
        }

        string line;
        getline(file, line); // Skip header

        while (getline(file, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string numberStr, roadName, budgetStr;

            getline(ss, numberStr, '\t');
            getline(ss, roadName, '\t');
            getline(ss, budgetStr);

            numberStr.erase(remove_if(numberStr.begin(), numberStr.end(), ::isspace), numberStr.end());
            roadName.erase(remove_if(roadName.begin(), roadName.end(), ::isspace), roadName.end());
            budgetStr.erase(remove_if(budgetStr.begin(), budgetStr.end(), ::isspace), budgetStr.end());

            // Clean up strings
            numberStr.erase(remove_if(numberStr.begin(), numberStr.end(), ::isspace), numberStr.end());
            roadName.erase(0, roadName.find_first_not_of(" \t"));
            roadName.erase(roadName.find_last_not_of(" \t") + 1);
            budgetStr.erase(0, budgetStr.find_first_not_of(" \t"));

            if (!numberStr.empty() && !roadName.empty() && !budgetStr.empty())
            {
                try
                {
                    // Parse city indices from road name
                    size_t dashPos = roadName.find('-');
                    if (dashPos != string::npos)
                    {
                        string city1Name = roadName.substr(0, dashPos);
                        string city2Name = roadName.substr(dashPos + 1);

                        int city1Index = findCityIndex(city1Name);
                        int city2Index = findCityIndex(city2Name);

                        if (city1Index != -1 && city2Index != -1)
                        {
                            roads.push_back(Road(stoi(numberStr), roadName, stod(budgetStr), city1Index, city2Index));
                        }
                    }
                }
                catch (const exception &)
                {
                    cout << "Warning: Invalid road data in file: " << line << "\n";
                }
            }
        }
        file.close();
    }

    void saveCitiesToFile()
    {
        ofstream file("cities.txt");
        if (!file.is_open())
        {
            cout << "Error: Cannot create cities.txt file.\n";
            return;
        }

        file << "Index\tCity_Name\n";
        for (const auto &city : cities)
        {
            file << city.index << "\t" << city.name << "\n";
        }
        file.close();
    }

    void saveRoadsToFile()
    {
        ofstream file("roads.txt");
        if (!file.is_open())
        {
            cout << "Error: Cannot create roads.txt file.\n";
            return;
        }

        file << "Nbr\tRoad\t\tBudget\n";
        for (const auto &road : roads)
        {
            file << road.number << ".\t" << road.roadName << "\t" << road.budget << "\n";
        }
        file.close();
    }

    int findCityIndex(const string &cityName)
    {
        for (size_t i = 0; i < cities.size(); i++)
        {
            if (cities[i].name == cityName)
            {
                return cities[i].index;
            }
        }
        return -1;
    }

    string findCityName(int index)
    {
        for (const auto &city : cities)
        {
            if (city.index == index)
            {
                return city.name;
            }
        }
        return "";
    }

    void updateMatrices()
    {
        int maxIndex = 0;
        for (const auto &city : cities)
        {
            maxIndex = max(maxIndex, city.index);
        }

        if (maxIndex == 0)
            return;

        // Initialize matrices
        adjacencyMatrix.assign(maxIndex + 1, vector<int>(maxIndex + 1, 0));
        budgetMatrix.assign(maxIndex + 1, vector<double>(maxIndex + 1, 0.0));

        // Fill matrices based on roads
        for (const auto &road : roads)
        {
            if (road.city1Index <= maxIndex && road.city2Index <= maxIndex)
            {
                adjacencyMatrix[road.city1Index][road.city2Index] = 1;
                adjacencyMatrix[road.city2Index][road.city1Index] = 1;
                budgetMatrix[road.city1Index][road.city2Index] = road.budget;
                budgetMatrix[road.city2Index][road.city1Index] = road.budget;
            }
        }
    }

    void addNewCities()
    {
        int numCities = getValidInteger("Enter number of cities to add: ", 1, 100);

        for (int i = 0; i < numCities; i++)
        {
            string cityName;
            bool validName = false;

            while (!validName)
            {
                cityName = getValidString("Enter name for city " + to_string(i + 1) + ": ");

                // Check if city name already exists
                bool nameExists = false;
                for (const auto &city : cities)
                {
                    if (city.name == cityName)
                    {
                        nameExists = true;
                        break;
                    }
                }

                if (nameExists)
                {
                    cout << "Error: A city with this name already exists. Please enter a different name.\n";
                }
                else
                {
                    validName = true;
                }
            }

            int newIndex = cities.empty() ? 1 : cities.back().index + 1;
            cities.push_back(City(newIndex, cityName));
            cout << "City '" << cityName << "' added with index " << newIndex << "\n";
        }

        updateMatrices();
        cout << numCities << " cities added successfully!\n";
    }

    void addRoadsBetweenCities()
    {
        if (cities.size() < 2)
        {
            cout << "Need at least 2 cities to create a road.\n";
            return;
        }

        displayCities();

        int city1Index, city2Index;

        while (true)
        {
            city1Index = getValidInteger("Enter index of first city: ");
            if (findCityName(city1Index).empty())
            {
                cout << "Error: City with index " << city1Index << " does not exist. Please try again.\n";
                continue;
            }
            break;
        }

        while (true)
        {
            city2Index = getValidInteger("Enter index of second city: ");
            if (findCityName(city2Index).empty())
            {
                cout << "Error: City with index " << city2Index << " does not exist. Please try again.\n";
                continue;
            }
            if (city1Index == city2Index)
            {
                cout << "Error: Cannot create a road from a city to itself. Please enter a different city index.\n";
                continue;
            }
            break;
        }

        string city1Name = findCityName(city1Index);
        string city2Name = findCityName(city2Index);

        // Check if road already exists
        for (const auto &road : roads)
        {
            if ((road.city1Index == city1Index && road.city2Index == city2Index) ||
                (road.city1Index == city2Index && road.city2Index == city1Index))
            {
                cout << "Road between these cities already exists.\n";
                return;
            }
        }

        string roadName = city1Name + "-" + city2Name;
        int roadNumber = roads.empty() ? 1 : roads.back().number + 1;

        roads.push_back(Road(roadNumber, roadName, 0.0, city1Index, city2Index));
        updateMatrices();

        cout << "Road '" << roadName << "' created successfully!\n";
    }

    void addBudgetForRoads()
    {
        if (roads.empty())
        {
            cout << "No roads available. Please add roads first.\n";
            return;
        }

        displayRoads();

        int roadNumber;
        bool roadFound = false;

        while (!roadFound)
        {
            roadNumber = getValidInteger("Enter road number to add/update budget: ", 1);

            for (auto &road : roads)
            {
                if (road.number == roadNumber)
                {
                    double budget = getValidDouble("Enter budget for road '" + road.roadName + "': ", 0.0);

                    road.budget = budget;
                    updateMatrices();
                    cout << "Budget updated successfully!\n";
                    roadFound = true;
                    break;
                }
            }

            if (!roadFound)
            {
                cout << "Error: Road number " << roadNumber << " not found. Please try again.\n";
            }
        }
    }

    void editCity()
    {
        if (cities.empty())
        {
            cout << "No cities available to edit.\n";
            return;
        }

        displayCities();

        int index;
        bool cityFound = false;

        while (!cityFound)
        {
            index = getValidInteger("Enter city index to edit: ");

            for (auto &city : cities)
            {
                if (city.index == index)
                {
                    string newName;
                    bool validName = false;

                    cout << "Current name: " << city.name << "\n";

                    while (!validName)
                    {
                        newName = getValidString("Enter new name: ");

                        // Check if new name already exists (excluding current city)
                        bool nameExists = false;
                        for (const auto &otherCity : cities)
                        {
                            if (otherCity.name == newName && otherCity.index != index)
                            {
                                nameExists = true;
                                break;
                            }
                        }

                        if (nameExists)
                        {
                            cout << "Error: A city with this name already exists. Please enter a different name.\n";
                        }
                        else
                        {
                            validName = true;
                        }
                    }

                    // Update road names that contain this city
                    string oldName = city.name;
                    for (auto &road : roads)
                    {
                        if (road.roadName.find(oldName) != string::npos)
                        {
                            size_t pos = road.roadName.find(oldName);
                            road.roadName.replace(pos, oldName.length(), newName);
                        }
                    }

                    city.name = newName;
                    cout << "City name updated successfully!\n";
                    cityFound = true;
                    break;
                }
            }

            if (!cityFound)
            {
                cout << "Error: City with index " << index << " not found. Please try again.\n";
            }
        }
    }

    void searchCityByIndex()
    {
        if (cities.empty())
        {
            cout << "No cities available.\n";
            return;
        }

        int index = getValidInteger("Enter city index to search: ");

        for (const auto &city : cities)
        {
            if (city.index == index)
            {
                cout << "\nCity found:\n";
                cout << "Index: " << city.index << "\n";
                cout << "Name: " << city.name << "\n";
                return;
            }
        }

        cout << "City with index " << index << " not found.\n";
    }

    void displayCities()
    {
        if (cities.empty())
        {
            cout << "No cities registered.\n";
            return;
        }

        cout << "\n=== CITIES LIST ===\n";
        cout << "Index\tCity_Name\n";
        cout << "-----\t---------\n";
        for (const auto &city : cities)
        {
            cout << city.index << "\t" << city.name << "\n";
        }
        cout << "\n";
    }

    void displayRoads()
    {
        if (roads.empty())
        {
            cout << "No roads registered.\n";
            return;
        }

        cout << "\n=== ROADS LIST ===\n";
        cout << "Nbr\tRoad\t\tBudget\n";
        cout << "---\t----\t\t------\n";
        for (const auto &road : roads)
        {
            cout << road.number << ".\t" << road.roadName << "\t\t" << road.budget << "\n";
        }

        displayAdjacencyMatrix();
        cout << "\n";
    }

    void displayAdjacencyMatrix()
    {
        if (cities.empty())
            return;

        cout << "\n=== ADJACENCY MATRIX ===\n";
        cout << "   ";
        for (const auto &city : cities)
        {
            cout << setw(8) << city.name.substr(0, 7);
        }
        cout << "\n";

        for (const auto &city1 : cities)
        {
            cout << setw(3) << city1.name.substr(0, 2);
            for (const auto &city2 : cities)
            {
                int value = 0;
                if (city1.index < adjacencyMatrix.size() && city2.index < adjacencyMatrix[city1.index].size())
                {
                    value = adjacencyMatrix[city1.index][city2.index];
                }
                cout << setw(8) << value;
            }
            cout << "\n";
        }
    }

    void displayBudgetMatrix()
    {
        if (cities.empty())
            return;

        cout << "\n=== BUDGET MATRIX ===\n";
        cout << "     ";
        for (const auto &city : cities)
        {
            cout << setw(10) << city.name.substr(0, 9);
        }
        cout << "\n";

        for (const auto &city1 : cities)
        {
            cout << setw(5) << city1.name.substr(0, 4);
            for (const auto &city2 : cities)
            {
                double value = 0.0;
                if (city1.index < budgetMatrix.size() && city2.index < budgetMatrix[city1.index].size())
                {
                    value = budgetMatrix[city1.index][city2.index];
                }
                cout << setw(10) << fixed << setprecision(1) << value;
            }
            cout << "\n";
        }
        cout << "\n";
    }

    void displayRecordedData()
    {
        cout << "\n=== ALL RECORDED DATA ===\n";
        displayCities();
        displayRoads();
        displayBudgetMatrix();
    }

    void showMenu()
    {
        cout << "\n=== CITIES AND ROADS MANAGEMENT SYSTEM ===\n";
        cout << "1. Add new city(ies)\n";
        cout << "2. Add roads between cities\n";
        cout << "3. Add the budget for roads\n";
        cout << "4. Edit city\n";
        cout << "5. Search for a city using its index\n";
        cout << "6. Display cities\n";
        cout << "7. Display roads\n";
        cout << "8. Display recorded data on console\n";
        cout << "9. Exit\n";
    }

    void run()
    {
        int choice;

        cout << "=========================================================================" << endl;
        cout << "**********=   WELCOME TO CITIES AND ROADS MANAGEMENT SYSTEM     =********" << endl;
        cout << "**********=            Developed by Remy Tresor - RCA           =********" << endl;
        cout << "=========================================================================" << endl;

        do
        {
            showMenu();
            choice = getValidMenuChoice(1, 9);

            switch (choice)
            {
            case 1:
                addNewCities();
                break;
            case 2:
                addRoadsBetweenCities();
                break;
            case 3:
                addBudgetForRoads();
                break;
            case 4:
                editCity();
                break;
            case 5:
                searchCityByIndex();
                break;
            case 6:
                displayCities();
                break;
            case 7:
                displayRoads();
                break;
            case 8:
                displayRecordedData();
                break;
            case 9:
                cout << "Thank you for using Cities and Roads Management System!\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
            }

        } while (choice != 9);
    }
};

int main()
{
    CitiesManagementSystem cms;
    cms.run();
    return 0;
}