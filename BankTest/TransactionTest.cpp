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

