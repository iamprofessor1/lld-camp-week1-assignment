#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <sstream>

using namespace std;

//================ Line Item ==================

struct LineItem
{
    string sku;
    int quantity{0};
    double unitPrice{0.0};
};

//================ Discount Strategy ==================

struct IDiscountStrategy
{
    virtual ~IDiscountStrategy() = default;
    virtual double apply(double subtotal, double value) const = 0;
};

class PercentageDiscount : public IDiscountStrategy
{
public:
    double apply(double subtotal, double value) const override
    {
        return subtotal * value / 100.0;
    }
};

class FlatDiscount : public IDiscountStrategy
{
public:
    double apply(double, double value) const override
    {
        return value;
    }
};

//================ Discount Factory ==================

class DiscountFactory
{
private:
    unordered_map<string, unique_ptr<IDiscountStrategy>> strategies;

public:
    DiscountFactory()
    {
        strategies["percent_off"] = make_unique<PercentageDiscount>();
        strategies["flat_off"] = make_unique<FlatDiscount>();
    }

    IDiscountStrategy *get(const string &name)
    {
        auto it = strategies.find(name);

        if (it == strategies.end())
            return nullptr;

        return it->second.get();
    }
};

//================ Invoice ==================

class Invoice
{
public:
    vector<LineItem> items;
    map<string, double> discounts;
    string email;

    double subtotal{0};
    double discountTotal{0};
    double tax{0};
    double grandTotal{0};

    Invoice(const vector<LineItem> &i,
            const map<string, double> &d,
            const string &e = "dummy@example.com")
        : items(i), discounts(d), email(e)
    {
    }
};

//================ Invoice Calculator ==================

class InvoiceCalculator
{
private:
    DiscountFactory &factory;

public:
    InvoiceCalculator(DiscountFactory &f)
        : factory(f)
    {
    }

    void calculate(Invoice &invoice)
    {
        invoice.subtotal = 0;

        for (const auto &item : invoice.items)
        {
            invoice.subtotal += item.quantity * item.unitPrice;
        }

        invoice.discountTotal = 0;

        for (const auto &d : invoice.discounts)
        {
            auto strategy = factory.get(d.first);

            if (strategy)
            {
                invoice.discountTotal +=
                    strategy->apply(invoice.subtotal, d.second);
            }
        }

        invoice.tax =
            (invoice.subtotal - invoice.discountTotal) * 0.18;

        invoice.grandTotal =
            invoice.subtotal -
            invoice.discountTotal +
            invoice.tax;
    }
};

//================ PDF ==================

class CreatePDF
{
public:
    string create(const Invoice &invoice)
    {
        ostringstream pdf;

        pdf << "========== INVOICE ==========\n\n";

        for (const auto &item : invoice.items)
        {
            pdf << item.sku
                << " x" << item.quantity
                << " @ " << item.unitPrice
                << '\n';
        }

        pdf << "\n-----------------------------\n";

        pdf << "Subtotal : " << invoice.subtotal << '\n';
        pdf << "Discount : " << invoice.discountTotal << '\n';
        pdf << "Tax      : " << invoice.tax << '\n';
        pdf << "Grand    : " << invoice.grandTotal << '\n';

        return pdf.str();
    }
};

//================ Email ==================

class EmailSender
{
public:
    void send(const Invoice &invoice)
    {
        if (!invoice.email.empty())
        {
            cout << "[SMTP] Sending invoice to "
                 << invoice.email << endl;
        }

        cout << "[LOG] Invoice processed. Total = "
             << invoice.grandTotal << endl;
    }
};

//================ Service ==================

class InvoiceService
{
private:
    InvoiceCalculator &calculator;
    CreatePDF &pdfGenerator;
    EmailSender &emailSender;

public:
    InvoiceService(InvoiceCalculator &c,
                   CreatePDF &p,
                   EmailSender &e)
        : calculator(c),
          pdfGenerator(p),
          emailSender(e)
    {
    }

    string process(const vector<LineItem> &items,
                   const map<string, double> &discounts,
                   const string &email = "dummy@example.com")
    {
        Invoice invoice(items, discounts, email);

        calculator.calculate(invoice);

        string pdf = pdfGenerator.create(invoice);

        emailSender.send(invoice);

        return pdf;
    }
};

//================ Main ==================

int main()
{
    vector<LineItem> items =
    {
        {"Laptop", 1, 50000},
        {"Mouse", 2, 500},
        {"Keyboard", 1, 2500}
    };

    map<string, double> discounts =
    {
        {"percent_off", 10},
        {"flat_off", 500}
    };

    DiscountFactory factory;
    InvoiceCalculator calculator(factory);
    CreatePDF pdf;
    EmailSender email;

    InvoiceService service(calculator, pdf, email);

    cout << service.process(
        items,
        discounts,
        "vikas@gmail.com");
}