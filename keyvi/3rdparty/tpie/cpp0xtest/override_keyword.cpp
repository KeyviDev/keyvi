class Base {
public:
	virtual ~Base();
	virtual void foo();
	virtual void baz();
};

class Derived : public Base {
public:
	void foo() override;
	void baz() final;
};

int main() { return 0; }
