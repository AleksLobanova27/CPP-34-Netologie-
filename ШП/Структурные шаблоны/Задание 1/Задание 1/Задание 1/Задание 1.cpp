#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

// Базовый компонент
class TextComponent {
public:
    virtual ~TextComponent() = default;
    virtual void render(const string& str) = 0;
};

// Конкретный компонент
class Text : public TextComponent {
public:
    void render(const string& str) override {
        cout << str;
    }
};

// Базовый декоратор
class TextDecorator : public TextComponent {
protected:
    TextComponent* component;
public:
    TextDecorator(TextComponent* comp) : component(comp) {}
    virtual ~TextDecorator() { delete component; }
};

// Декоратор для параграфа
class Paragraph : public TextDecorator {
public:
    Paragraph(TextComponent* comp) : TextDecorator(comp) {}

    void render(const string& str) override {
        cout << "<p>";
        component->render(str);
        cout << "</p>";
    }
};

// Декоратор для перевернутого текста
class Reversed : public TextDecorator {
public:
    Reversed(TextComponent* comp) : TextDecorator(comp) {}

    void render(const string& str) override {
        string reversed = str;
        reverse(reversed.begin(), reversed.end());
        component->render(reversed);
    }
};

// Декоратор для ссылки - особый случай, сигнатура render отличается
class Link : public TextComponent {
private:
    TextComponent* component;
public:
    Link(TextComponent* comp) : component(comp) {}
    ~Link() { delete component; }

    void render(const string& href, const string& text) {
        cout << "<a href=" << href << ">";
        component->render(text);
        cout << "</a>";
    }

    void render(const string& str) override {
        render("#", str); 
    }
};

int main() {
    // Параграф
    auto text_block = new Paragraph(new Text());
    text_block->render("Hello world");
    cout << endl << endl;
    delete text_block;

    // Перевернутый текст
    auto reversed_block = new Reversed(new Text());
    reversed_block->render("Hello world");
    cout << endl << endl;
    delete reversed_block;

    // Ссылка
    auto link_block = new Link(new Text());
    link_block->render("netology.ru", "Hello world");
    cout << endl;
    delete link_block;

    return 0;
}