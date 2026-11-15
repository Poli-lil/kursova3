
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <ctime>

using namespace std;

// Класс кошелька
class Wallet {
public:
    int id;
    string name;
    string type;
    double balance;

    Wallet(int i, string n, string t, double b = 0)
        : id(i), name(n), type(t), balance(b) {
    }

    void addMoney(double amount) {
        balance += amount;
    }

    void removeMoney(double amount) {
        balance -= amount;
    }

    void show() const {
        cout << id << ". " << name << " (" << type
            << ") - " << fixed << setprecision(2)
            << balance << " грн\n";
    }
};

// Класс финансовой операции
class Transaction {
public:
    int id;
    int walletId;
    string date;
    string type;
    string category;
    string description;
    double amount;

    Transaction(int i, int w, string d, string t,
        string c, string desc, double a)
        : id(i), walletId(w), date(d), type(t),
        category(c), description(desc), amount(a) {
    }
};

// Основной класс управления финансами
class FinanceManager {
private:
    vector<Wallet> wallets;
    vector<Transaction> transactions;

    int nextWalletId = 1;
    int nextTransactionId = 1;

    string currentDate() {
        time_t now = time(0);
        tm* timeInfo = localtime(&now);

        char buffer[11];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeInfo);

        return buffer;
    }

    string getDate(int daysAgo) {
        time_t now = time(0);
        now -= daysAgo * 86400;

        tm* timeInfo = localtime(&now);

        char buffer[11];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeInfo);

        return buffer;
    }

    Wallet* findWallet(int id) {
        for (auto& wallet : wallets) {
            if (wallet.id == id)
                return &wallet;
        }
        return nullptr;
    }

    void saveData() {
        ofstream file("wallets.txt");

        for (const auto& w : wallets) {
            file << w.id << "|" << w.name << "|"
                << w.type << "|" << w.balance << "\n";
        }

        file.close();

        ofstream file2("transactions.txt");

        for (const auto& t : transactions) {
            file2 << t.id << "|" << t.walletId << "|"
                << t.date << "|" << t.type << "|"
                << t.category << "|" << t.description
                << "|" << t.amount << "\n";
        }

        file2.close();
    }

    void loadData() {
        ifstream file("wallets.txt");
        string line;

        while (getline(file, line)) {
            stringstream ss(line);

            string id, name, type, balance;

            getline(ss, id, '|');
            getline(ss, name, '|');
            getline(ss, type, '|');
            getline(ss, balance, '|');

            if (!id.empty() && !balance.empty()) {
                int number = stoi(id);

                wallets.emplace_back(
                    number, name, type, stod(balance)
                );

                nextWalletId = max(nextWalletId, number + 1);
            }
        }

        file.close();

        ifstream file2("transactions.txt");

        while (getline(file2, line)) {
            stringstream ss(line);

            string id, walletId, date, type;
            string category, description, amount;

            getline(ss, id, '|');
            getline(ss, walletId, '|');
            getline(ss, date, '|');
            getline(ss, type, '|');
            getline(ss, category, '|');
            getline(ss, description, '|');
            getline(ss, amount, '|');

            if (!id.empty() && !amount.empty()) {
                int number = stoi(id);

                transactions.emplace_back(
                    number, stoi(walletId), date, type,
                    category, description, stod(amount)
                );

                nextTransactionId =
                    max(nextTransactionId, number + 1);
            }
        }

        file2.close();
    }

    void addTransaction(string type) {
        if (wallets.empty()) {
            cout << "Сначала создайте кошелек!\n";
            return;
        }

        showWallets();

        int walletId;
        cout << "Введите ID кошелька: ";
        cin >> walletId;
        cin.ignore();

        Wallet* wallet = findWallet(walletId);

        if (!wallet) {
            cout << "Кошелек не найден!\n";
            return;
        }

        double amount;
        cout << "Введите сумму: ";
        cin >> amount;
        cin.ignore();

        if (amount <= 0) {
            cout << "Сумма должна быть положительной!\n";
            return;
        }

        string category, description, date;

        cout << "Введите категорию: ";
        getline(cin, category);

        cout << "Введите описание: ";
        getline(cin, description);

        cout << "Дата (ГГГГ-ММ-ДД, Enter - сегодня): ";
        getline(cin, date);

        if (date.empty())
            date = currentDate();

        if (type == "Расход")
            wallet->removeMoney(amount);
        else
            wallet->addMoney(amount);

        transactions.emplace_back(
            nextTransactionId++, walletId, date,
            type, category, description, amount
        );

        saveData();

        cout << "Операция добавлена!\n";
    }

    void topExpenses(string from, string to) {
        vector<Transaction> result;

        for (const auto& t : transactions) {
            if (t.type == "Расход" &&
                t.date >= from && t.date <= to) {
                result.push_back(t);
            }
        }

        sort(result.begin(), result.end(),
            [](const Transaction& a, const Transaction& b) {
                return a.amount > b.amount;
            });

        cout << "\nТОП-3 расходов:\n";

        for (int i = 0; i < min(3, (int)result.size()); i++) {
            cout << i + 1 << ". " << result[i].category
                << " - " << result[i].amount << " грн\n";
        }
    }

    void topCategories(string from, string to) {
        map<string, double> categories;

        for (const auto& t : transactions) {
            if (t.type == "Расход" &&
                t.date >= from && t.date <= to) {
                categories[t.category] += t.amount;
            }
        }

        vector<pair<string, double>> result(
            categories.begin(), categories.end()
        );

        sort(result.begin(), result.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

        cout << "\nТОП-3 категорий расходов:\n";

        for (int i = 0; i < min(3, (int)result.size()); i++) {
            cout << i + 1 << ". " << result[i].first
                << " - " << result[i].second << " грн\n";
        }
    }

public:
    FinanceManager() {
        loadData();
    }

    ~FinanceManager() {
        saveData();
    }

    void createWallet() {
        string name, type;

        cout << "Название кошелька: ";
        getline(cin, name);

        cout << "Тип (Кошелек / Дебетовая карта / Кредитная карта): ";
        getline(cin, type);

        wallets.emplace_back(nextWalletId++, name, type);

        saveData();

        cout << "Кошелек создан!\n";
    }

    void showWallets() {
        cout << "\n--- МОИ КОШЕЛЬКИ ---\n";

        for (const auto& wallet : wallets)
            wallet.show();
    }

    void replenish() {
        if (wallets.empty()) {
            cout << "Сначала создайте кошелек!\n";
            return;
        }

        showWallets();

        int id;
        double amount;

        cout << "ID кошелька: ";
        cin >> id;

        cout << "Сумма пополнения: ";
        cin >> amount;
        cin.ignore();

        Wallet* wallet = findWallet(id);

        if (!wallet || amount <= 0) {
            cout << "Ошибка!\n";
            return;
        }

        wallet->addMoney(amount);

        transactions.emplace_back(
            nextTransactionId++, id, currentDate(),
            "Пополнение", "Пополнение",
            "Пополнение кошелька", amount
        );

        saveData();

        cout << "Баланс пополнен!\n";
    }

    void showHistory() {
        cout << "\n--- ИСТОРИЯ ОПЕРАЦИЙ ---\n";

        for (const auto& t : transactions) {
            cout << t.date << " | " << t.type
                << " | " << t.category << " | "
                << t.amount << " грн | "
                << t.description << "\n";
        }
    }

    void report(int period) {
        string today = currentDate();
        string from;

        if (period == 1)
            from = today;
        else if (period == 2)
            from = getDate(6);
        else {
            from = today;
            from[8] = '0';
            from[9] = '1';
        }

        double income = 0;
        double expense = 0;

        cout << "\n--- ФИНАНСОВЫЙ ОТЧЕТ ---\n";

        for (const auto& t : transactions) {
            if (t.date >= from && t.date <= today) {
                if (t.type == "Доход")
                    income += t.amount;

                if (t.type == "Расход")
                    expense += t.amount;

                cout << t.date << " | " << t.category
                    << " | " << t.type << " | "
                    << t.amount << " грн\n";
            }
        }

        cout << "\nДоходы: " << income << " грн\n";
        cout << "Расходы: " << expense << " грн\n";
        cout << "Разница: " << income - expense << " грн\n";

        topExpenses(from, today);
        topCategories(from, today);

        ofstream file("report.txt");

        file << "Отчет с " << from << " по " << today << "\n";
        file << "Доходы: " << income << "\n";
        file << "Расходы: " << expense << "\n";
        file << "Разница: " << income - expense << "\n";

        file.close();

        cout << "\nОтчет сохранен в report.txt\n";
    }

    void menu() {
        int choice;

        do {
            cout << "\n===== УПРАВЛЕНИЕ ФИНАНСАМИ =====\n";
            cout << "1. Создать кошелек\n";
            cout << "2. Показать кошельки\n";
            cout << "3. Пополнить кошелек\n";
            cout << "4. Добавить доход\n";
            cout << "5. Добавить расход\n";
            cout << "6. История операций\n";
            cout << "7. Отчет за день\n";
            cout << "8. Отчет за неделю\n";
            cout << "9. Отчет за месяц\n";
            cout << "0. Выход\n";
            cout << "Выбор: ";

            cin >> choice;
            cin.ignore();

            switch (choice) {
            case 1:
                createWallet();
                break;
            case 2:
                showWallets();
                break;
            case 3:
                replenish();
                break;
            case 4:
                addTransaction("Доход");
                break;
            case 5:
                addTransaction("Расход");
                break;
            case 6:
                showHistory();
                break;
            case 7:
                report(1);
                break;
            case 8:
                report(2);
                break;
            case 9:
                report(3);
                break;
            case 0:
                cout << "Выход из программы.\n";
                break;
            default:
                cout << "Неверный выбор!\n";
            }

        } while (choice != 0);
    }
};

int main() {
    setlocale(LC_ALL, "");

    FinanceManager manager;
    manager.menu();

    return 0;
}
