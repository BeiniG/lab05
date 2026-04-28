# Лабораторная работа №5
## Homework
### Тестирование библиотеки Banking

[![Linux CI](https://github.com/BeiniG/lab05/actions/workflows/CI.yml/badge.svg)](https://github.com/BeiniG/lab05/actions/workflows/CI.yml)
[![Coverage Status](https://coveralls.io/repos/github/BeiniG/lab05/badge.svg)](https://coveralls.io/github/BeiniG/lab05)

Файл для тестирования аккаунтов:
```cpp
#include <Account.h>
#include <gtest/gtest.h>

class AccountFixture : public testing::Test {
public:
	Account* acc;
	void SetUp() { acc = new Account(1, 1000); }
	void TearDown() { delete acc; }
};

TEST_F(AccountFixture, GetID) {
	EXPECT_EQ(acc->id(), 1);
}

TEST_F(AccountFixture, GetBalance) {
	EXPECT_EQ(acc->GetBalance(), 1000);
}

TEST_F(AccountFixture, ChangeBalanceTest) {
	acc->Lock();
	acc->ChangeBalance(500);
	EXPECT_EQ(acc->GetBalance(), 1500);
}

TEST_F(AccountFixture, ChangeBalanceNegativeTest) {
	acc->Lock();
	acc->ChangeBalance(-200);
	EXPECT_EQ(acc->GetBalance(), 800);
}

TEST_F(AccountFixture, FailedBalanceTest) {
	EXPECT_THROW(acc->ChangeBalance(500), std::runtime_error);
}

TEST_F(AccountFixture, LockUnlockTest) {
	acc->Lock();
	acc->Unlock();
	EXPECT_NO_THROW(acc->Lock());
}

TEST_F(AccountFixture, DoubleLockTest) {
	acc->Lock();
	EXPECT_THROW(acc->Lock(), std::runtime_error);
}


```

Файл для тестирования трансакций:
```cpp
#include <Account.h>
#include <Transaction.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class AccountMock : public Account {
public:
	AccountMock(int id, int balance) : Account(id, balance) {}
	
	MOCK_METHOD(int, GetBalance, (), (const, override));
  	MOCK_METHOD(void, ChangeBalance, (int diff), (override));
  	MOCK_METHOD(void, Lock, (), (override));
  	MOCK_METHOD(void, Unlock, (), (override));
};

class TransactionFixture : public testing::Test {
public:
	Transaction* tr;
	AccountMock* from;
	AccountMock* to;
	void SetUp () override { 
		tr = new Transaction;
		from = new testing::NiceMock<AccountMock>(1, 1000);
		to = new testing::NiceMock<AccountMock>(2, 1000);
	}
	void TearDown () override { 
		delete tr;
		delete from;
		delete to;
	}
};

TEST_F(TransactionFixture, Fee) {
	EXPECT_EQ(tr->fee(), 1);
}

TEST_F(TransactionFixture, ChangeFee) {
	tr->set_fee(10);
	EXPECT_EQ(tr->fee(), 10);
}

TEST_F(TransactionFixture, TransferTest) {
	EXPECT_CALL(*to, ChangeBalance(100)).Times(1);
	EXPECT_CALL(*from, GetBalance()).WillOnce(testing::Return(1000)).WillRepeatedly(testing::Return(899));
	EXPECT_CALL(*to, GetBalance()).WillRepeatedly(testing::Return(1100));

	EXPECT_TRUE(tr->Make(*from, *to, 100));
}

TEST_F(TransactionFixture, SelfTransfer) {
	EXPECT_THROW(tr->Make(*from, *from, 100), std::logic_error);
}

TEST_F(TransactionFixture, NegativeTransfer) {
	EXPECT_THROW(tr->Make(*from, *to, -200), std::invalid_argument);
}
TEST_F(TransactionFixture, BigSumTest) {
	EXPECT_CALL(*from, GetBalance()).WillRepeatedly(testing::Return(1000));
	EXPECT_CALL(*to, ChangeBalance(1200)).Times(1);
	EXPECT_CALL(*to, ChangeBalance(-1200)).Times(1);
	EXPECT_CALL(*to, GetBalance()).WillRepeatedly(testing::Return(1000));
	EXPECT_FALSE(tr->Make(*from, *to, 1200));
}
TEST_F(TransactionFixture, SmallSumTest) {
	EXPECT_THROW(tr->Make(*from, *to, 30), std::logic_error);
}

TEST_F(TransactionFixture, BigFeeTest) {
	tr->set_fee(600);
	EXPECT_FALSE(tr->Make(*from, *to, 100));
}
```
`CMakeLists.txt` для папки banking:
```cmake
cmake_minimum_required(VERSION 3.10)
project(banking)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT TARGET account)
	add_library(account STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Account.cpp)
endif()

if (NOT TARGET transaction)
	add_library(transaction STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Transaction.cpp)
endif()

target_include_directories(account PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(transaction PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(transaction PUBLIC account)

if (COLLECT_COVERAGE)
	target_compile_options(account PRIVATE -O0 -g --coverage)
	target_link_options(account PRIVATE --coverage)
	target_compile_options(transaction PRIVATE -O0 -g --coverage)
	target_link_options(transaction PRIVATE --coverage)
endif()
```
Общий `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.10)
project(banking_tests)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)
option(COLLECT_COVERAGE "Collect coverage" OFF)

add_subdirectory(banking)

if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(third_party/gtest)
	
	file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/BankTest/*.cpp")
	add_executable(BankTesting ${TEST_SOURCES})
	target_link_libraries(BankTesting account transaction gtest_main gmock_main)

	if (COLLECT_COVERAGE)
		target_link_libraries(BankTesting gcov)
		target_compile_options(BankTesting PRIVATE -O0 -g --coverage)
		target_link_options(BankTesting PRIVATE --coverage)
	endif()

	add_test(NAME BankTesting COMMAND BankTesting)
	endif()
```
Запускаем cmake при помощи команды

```sh
cmake -H. -B build -D BUILD_TESTS=ON -D COLLECT_COVERAGE=ON
```

<details>
<summary>Вывод</summary>

```
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/vboxuser/lab05/build
```

</details>
Собираем проект при помощи команды:

```sh
cmake --build build
```

<details>
<summary>Вывод </summary>

```sh
[  6%] Building CXX object third_party/gtest/googletest/CMakeFiles/gtest.dir/src/gtest-all.cc.o
[ 13%] Linking CXX static library ../../../lib/libgtest.a
[ 13%] Built target gtest
[ 20%] Building CXX object third_party/gtest/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
[ 26%] Linking CXX static library ../../../lib/libgtest_main.a
[ 26%] Built target gtest_main
[ 33%] Building CXX object banking/CMakeFiles/account.dir/Account.cpp.o
[ 40%] Linking CXX static library libaccount.a
[ 40%] Built target account
[ 46%] Building CXX object banking/CMakeFiles/transaction.dir/Transaction.cpp.o
[ 53%] Linking CXX static library libtransaction.a
[ 53%] Built target transaction
[ 60%] Building CXX object third_party/gtest/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o
[ 66%] Linking CXX static library ../../../lib/libgmock.a
[ 66%] Built target gmock
[ 73%] Building CXX object third_party/gtest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.o
[ 80%] Linking CXX static library ../../../lib/libgmock_main.a
[ 80%] Built target gmock_main
[ 86%] Building CXX object CMakeFiles/BankTesting.dir/BankTest/AccountTest.cpp.o
[ 93%] Building CXX object CMakeFiles/BankTesting.dir/BankTest/TransactionTest.cpp.o
[100%] Linking CXX executable BankTesting
[100%] Built target BankTesting
```

</details>
Тестируем при помощи команды 

```sh
build/BankTesting
```

<details>
<summary>Вывод </summary>

```sh
Running main() from /home/vboxuser/lab05/third_party/gtest/googletest/src/gtest_main.cc
[==========] Running 15 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 7 tests from AccountFixture
[ RUN      ] AccountFixture.GetID
[       OK ] AccountFixture.GetID (0 ms)
[ RUN      ] AccountFixture.GetBalance
[       OK ] AccountFixture.GetBalance (0 ms)
[ RUN      ] AccountFixture.ChangeBalanceTest
[       OK ] AccountFixture.ChangeBalanceTest (0 ms)
[ RUN      ] AccountFixture.ChangeBalanceNegativeTest
[       OK ] AccountFixture.ChangeBalanceNegativeTest (0 ms)
[ RUN      ] AccountFixture.FailedBalanceTest
[       OK ] AccountFixture.FailedBalanceTest (2 ms)
[ RUN      ] AccountFixture.LockUnlockTest
[       OK ] AccountFixture.LockUnlockTest (0 ms)
[ RUN      ] AccountFixture.DoubleLockTest
[       OK ] AccountFixture.DoubleLockTest (0 ms)
[----------] 7 tests from AccountFixture (2 ms total)

[----------] 8 tests from TransactionFixture
[ RUN      ] TransactionFixture.Fee
[       OK ] TransactionFixture.Fee (0 ms)
[ RUN      ] TransactionFixture.ChangeFee
[       OK ] TransactionFixture.ChangeFee (0 ms)
[ RUN      ] TransactionFixture.TransferTest
1 send to 2 $100
Balance 1 is 899
Balance 2 is 1100
[       OK ] TransactionFixture.TransferTest (0 ms)
[ RUN      ] TransactionFixture.SelfTransfer
[       OK ] TransactionFixture.SelfTransfer (0 ms)
[ RUN      ] TransactionFixture.NegativeTransfer
[       OK ] TransactionFixture.NegativeTransfer (0 ms)
[ RUN      ] TransactionFixture.BigSumTest
1 send to 2 $1200
Balance 1 is 1000
Balance 2 is 1000
[       OK ] TransactionFixture.BigSumTest (0 ms)
[ RUN      ] TransactionFixture.SmallSumTest
[       OK ] TransactionFixture.SmallSumTest (0 ms)
[ RUN      ] TransactionFixture.BigFeeTest
[       OK ] TransactionFixture.BigFeeTest (0 ms)
[----------] 8 tests from TransactionFixture (0 ms total)

[----------] Global test environment tear-down
[==========] 15 tests from 2 test suites ran. (3 ms total)
[  PASSED  ] 15 tests.
```

</details>

Из вывода видно, что сборка и тесты прошли успешно. Теперь нужно воспользоваться `Coveralls`. Заходим на сайт, подключаем репозиторий и получаем токен `COVERALLS_REPO_TOKEN`, который добавляем в раздел "Secrets" в настройках репозитория. После чего создаем `CI.yml`:

```yml
name: BankTest
on: push
jobs:
  build_and_test:
    runs-on: ubuntu-latest
    steps:
      - name: Set GCC 11
        uses: egor-tensin/setup-gcc@v1
        with:
          version: 11
          platform: x64
      - name: Get repo files
        uses: actions/checkout@v6
        with:
          submodules: true
      - name: Build configuration
        run: cmake -H. -B build -D BUILD_TESTS=ON -D COLLECT_COVERAGE=ON
      - name: Build project
        run: cmake --build build
      - name: Run tests
        run: ./build/BankTesting
      - name: Install lcov
        run: sudo apt install lcov
      - name: Create a conf file
        run: lcov --capture --directory build/banking --gcov-tool /usr/bin/gcov-11 --output-file=cov.info
      - name: Cover percent
        run: lcov --list cov.info
      - name: Coveralls Upload
        uses: coverallsapp/github-action@v2
        with:
          github-token: ${{ secrets.GITHUB_TOKEN }}
          file: cov.info
```
Заходим на сайт `Coveralls`, копируем ссылки из раздела badge и вставляем их в README.md
