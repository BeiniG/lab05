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


