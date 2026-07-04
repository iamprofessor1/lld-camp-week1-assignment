// 03-notify-dip-ocp.cpp
#include <iostream>
#include <string>

using namespace std;

struct User
{
    string email;
    string phone;
};

struct IMailer
{
    virtual void send(const string &templ, const string &to, const string &body) = 0;
};
class SmtpMailer : public IMailer
{
public:
    void send(const string &templ, const string &to, const string &body) override
    {
        cout << "[SMTP] template=" << templ << " to=" << to << " body=" << body << "\n";
    }
};

struct ISms
{
    virtual void sendOTP(const string &phone, const string &code) = 0;
};

class TwilioClient : public ISms
{
public:
    void sendOTP(const string &phone, const string &code) override
    {
        cout << "[Twilio] OTP " << code << " -> " << phone << "\n";
    }
};

class SignUpService
{
    IMailer *mailer;
    ISms *sms;

public:
    SignUpService(IMailer *m, ISms *s) : mailer(m), sms(s) {}
    bool signUp(const User &u)
    {
        if (u.email.empty())
            return false;
        // pretend DB save here…

        // hard-coded providers
        sendEmail(u);
        sendOtp(u);

        return true;
    }
    void sendEmail(const User &u)
    {
        mailer->send("welcome", u.email, "Welcome!");
    }
    void sendOtp(const User &u)
    {
        sms->sendOTP(u.phone, "123456");
    }
};

int main()
{
    SmtpMailer mail;
    TwilioClient sms;
    SignUpService svc(&mail, &sms);
    svc.signUp({"user@example.com", "+15550001111"});
    return 0;
}
