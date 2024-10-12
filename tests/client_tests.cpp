#include <gtest/gtest.h> 
#include "../src/user.h"

TEST(ClientTest, Deposit) {
    User client(1, "Sigma", 1234); 
    client.deposit(100); 
    
    EXPECT_EQ(client.get_balance(), 100); 
}

TEST(ClientTest, Withdraw) {
    User client(1, "Sigma", 1234); 
    client.deposit(100); 
    client.withdraw(50, 1234); 

    EXPECT_EQ(client.get_balance(), 50); 
}

TEST(ClientTest, WithdrawInvalidPin) {
    User client(1, "Sigma", 1234); 
    client.deposit(100); 

    EXPECT_THROW(client.withdraw(50, 60), std::invalid_argument); 
    EXPECT_EQ(client.get_balance(), 100); 
}

TEST(ClientTest, Send) 
{
    User main(1, "sigma", 1234); 
    main.deposit(100); 

    User receive(2, "thug", 12345); 

    main.send(50, receive, 1234);

    EXPECT_EQ(receive.get_balance(), 50);
    EXPECT_EQ(main.get_balance(), 50);
}

TEST(ClientTest, NegativeDeposit) {
    User client(1, "Sigma", 1234); 

    EXPECT_THROW(client.deposit(-100), std::invalid_argument); 
    EXPECT_EQ(client.get_balance(), 0); 
}

TEST(ClientTest, NegativeWithdrawal) {
    User client(1, "Sigma", 1234); 
    client.deposit(100); 

    EXPECT_THROW(client.withdraw(-50, 1234), std::invalid_argument); 
    EXPECT_EQ(client.get_balance(), 100); 
}

TEST(ClientTest, InsufficientFunds) {
    User client(1, "Sigma", 1234); 
    client.deposit(50); 

    EXPECT_THROW(client.withdraw(100, 1234), std::runtime_error); 
    EXPECT_EQ(client.get_balance(), 50); 
}

TEST(ClientTest, SendInvalidPin) {
    User main(1, "sigma", 1234); 
    main.deposit(100); 

    User receive(2, "thug", 12345); 

    EXPECT_THROW(main.send(50, receive, 1111), std::invalid_argument);
    EXPECT_EQ(main.get_balance(), 100);
    EXPECT_EQ(receive.get_balance(), 0);
}

TEST(ClientTest, SendInsufficientFunds) {
    User main(1, "sigma", 1234); 
    main.deposit(50); 

    User receive(2, "thug", 12345); 

    EXPECT_THROW(main.send(100, receive, 1234), std::runtime_error);
    EXPECT_EQ(main.get_balance(), 50);
    EXPECT_EQ(receive.get_balance(), 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS(); 
}