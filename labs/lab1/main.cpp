#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
using BoxedMutex = std::unique_ptr<std::mutex>;
using MutexGuard = std::unique_lock<std::mutex>;

constexpr size_t NUM_SALE_THREADS = 8;
constexpr size_t NUM_SALES_PER_THREAD = 20;
constexpr size_t NUM_SALE_CHECKS = 20;
constexpr auto SALE_INTERVAL = 3s;
constexpr auto SALE_CHECK_INTERVAL = 3s;

struct Product {
    std::string name;
    int32_t price;
};

struct Stock {
    Product product;
    int32_t quantity;
    BoxedMutex quantity_mutex;
};

struct BillItem {
    size_t stock_index;
    int32_t quantity;
};

struct Bill {
    std::vector<BillItem> items;
    int32_t price = 0;
};

struct Shop {
    std::vector<Stock> stocks;
    std::mutex sales_lock;
    std::vector<Bill> bills;
    int32_t money = 0;

    void add_stock(const Product& product, int32_t quantity) {
        stocks.push_back({ product, quantity, std::make_unique<std::mutex>() });
    }
};

void print_sale(const Shop& shop, const Bill& bill) {
    std::cout
        << "[SALE OF "
        << "$" << bill.price
        << " FROM "
        << std::this_thread::get_id()
        << "]\n";

    for (const auto& item : bill.items) {
        const auto& stock = shop.stocks[item.stock_index];

        std::cout
            << item.quantity
            << " x "
            << stock.product.name
            << " = $"
            << item.quantity * stock.product.price
            << '\n';
    }

    std::cout << '\n';
}

void sell_random_stocks(Shop& shop) {
    if (shop.stocks.empty()) {
        return;
    }

    std::random_device random_device;
    std::mt19937 random_engine(random_device());
    std::uniform_int_distribution<size_t> sell_attempts_range(1, shop.stocks.size());
    std::uniform_int_distribution<size_t> stock_index_range(0, shop.stocks.size() - 1);

    for (size_t i = 0; i < NUM_SALES_PER_THREAD; ++i, std::this_thread::sleep_for(SALE_INTERVAL)) {
        std::vector<size_t> stock_indexes;
        std::vector<MutexGuard> quantity_guards;

        // Choose the stocks to sell
        {
            size_t sell_attempts = sell_attempts_range(random_engine);

            for (size_t j = 0; j < sell_attempts; ++j) {
                size_t stock_index = stock_index_range(random_engine);
                stock_indexes.push_back(stock_index);
            }

            // Sort stocks and remove duplicates
            std::sort(stock_indexes.begin(), stock_indexes.end());
            auto unique_it = std::unique(stock_indexes.begin(), stock_indexes.end());
            stock_indexes.erase(unique_it, stock_indexes.end());
        }

        Bill bill;

        // Create a bill without altering the shop state
        {
            for (size_t i : stock_indexes) {
                quantity_guards.push_back(MutexGuard(*shop.stocks[i].quantity_mutex));
            }

            for (size_t i : stock_indexes) {
                auto& stock = shop.stocks[i];

                if (stock.quantity == 0) {
                    continue;
                }

                int32_t max_quantity = std::min(5, stock.quantity);
                std::uniform_int_distribution<int32_t> quantity_range(1, max_quantity);
                int32_t quantity = quantity_range(random_engine);
                int32_t price = quantity * stock.product.price;

                bill.items.push_back({ i, quantity });
                bill.price += price;
            }
        }

        // Perform the transaction by subtracting stocks, adding the bill and increasing money
        {
            MutexGuard sales_guard(shop.sales_lock);

            if (bill.price != 0) {
                for (const auto& item : bill.items) {
                    shop.stocks[item.stock_index].quantity -= item.quantity;
                }

                print_sale(shop, bill);

                shop.money += bill.price;
                shop.bills.push_back(std::move(bill));
            }
        }
    }
}

void check_sales(Shop& shop) {
    for (size_t i = 0; i < NUM_SALE_CHECKS; ++i, std::this_thread::sleep_for(SALE_CHECK_INTERVAL)) {
        std::vector<MutexGuard> quantity_guards;

        // Lock all stocks
        for (const auto& stock : shop.stocks) {
            quantity_guards.push_back(MutexGuard(*stock.quantity_mutex));
        }

        {
            MutexGuard sales_guard(shop.sales_lock);
            int32_t shop_money = 0;

            for (const auto& bill : shop.bills) {
                int32_t bill_price = 0;

                for (const auto& item : bill.items) {
                    int32_t product_price = shop.stocks[item.stock_index].product.price;
                    bill_price += item.quantity * product_price;
                }

                if (bill_price != bill.price) {
                    throw std::runtime_error("Invalid bill");
                }

                shop_money += bill_price;
            }

            if (shop_money != shop.money) {
                throw std::runtime_error("Invalid sales");
            }

            std::cout << "[INVENTORY CHECK FROM " << std::this_thread::get_id() << "]\n";

            // Print the shop's state
            std::cout << "SHOP'S STATE:\n"
                << "STOCKS:\n";
            for (const auto& stock : shop.stocks) {
                std::cout << "\t" << stock.quantity << " x " << stock.product.name << "\n";
            }
            std::cout << "\n";
        }
    }
}

int main() {
    Shop shop;
    shop.add_stock({ "salt", 10 }, 100);
    shop.add_stock({ "coffee", 20 }, 500);
    shop.add_stock({ "bread", 5 }, 200);
    shop.add_stock({ "potatoes", 40 }, 100);
    shop.add_stock({ "bananas", 60 }, 200);

    std::vector<std::thread> sale_threads;

    for (size_t i = 0; i < NUM_SALE_THREADS; ++i) {
        sale_threads.push_back(std::thread([&shop]() {
            sell_random_stocks(shop);
        }));
    }

    std::thread check_thread([&shop]() {
        check_sales(shop);
    });

    for (auto& sale_thread : sale_threads) {
        if (sale_thread.joinable()) {
            sale_thread.join();
        }
    }

    if (check_thread.joinable()) {
        check_thread.join();
    }

    return 0;
}