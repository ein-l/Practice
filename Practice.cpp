#include <iostream>
#include <functional> //std::function

//VS�Ō��m�ł��Ȃ������̂ť�����
#define CPP14

//
#if   __cplusplus >= 201402L
// someting to do for C++14
#define CPP14
#elif __cplusplus >= 201103L
// someting to do for C++11
#define CPP11
#elif __cplusplus >= 199711L
// someting to do for C++
#else
// someting to do for C
#endif


template<typename T>
void f(T&& param);

class TypeA {
	int a;
	int b;
public:
	TypeA(){
		a = 0;
		b = 1;
		Display();
	}
	void NoReferenceFunction(){
		std::cout << "no reference" << std::endl;
	}
	void Update(){
		a = a + b;
	}
	void Display(){
		std::cout << "A:" << a << std::endl;
		printf("%x \n", &a);
	}
};

class TypeB {
	int c;
	int d;
public:
	TypeB(){
		c = 2;
		d = 3;
	}
	void Display(){
		std::cout << "C:" << c << std::endl;
	}
};

class Increment {
	int number;
public:
	Increment(int n){
		number = n;
	}
	template<typename T>
	void AddNumber(T num){
		number += num;
		std::cout << "Number" << number << std::endl;
	}
	//Increment() = delete;
	void AddNumber(float) = delete; //delete�̃e�X�g�I
	int GetNumber(){
		return number;
	}
};
//C++14�H
//template<> void Increment::AddNumber<bool>(bool) = delete;

//override�̓X�L�b�v

//MoveSemantics
class MoveClass {
public :
	//�󂯎�������̂ƃA�h���X���ς��Ȃ����̂�������(move��move)
	//universal�ł���K�v������
	template<typename T>
	static void Relay(T&& val) {
		val.Update(); //����͔��f����Ȃ�
		val = std::forward<T>(T());
	};

	//���[�J���Q�Ƃ̃A�h���X�ƈ�v�������̂��A��(�������Q�Ƃ͉���I)
	template<typename T>
	static T&& Build() {
		T v = T();
		v.Update();
		return std::forward<T>(v);
	};
};

template <typename T>
class Transpoter {
	T Target;
public:
	Transpoter(T f){
		Target = f;
	}

	template<typename... Ts>
	void Transfar(Ts&&... params){
		Target(std::forward<Ts>(params)...);
	}
};

int main(void)
{
	//�p�����[�^�̌^���_�e�X�g
	int x = 27;
	TypeA ta;
	TypeB&& tb = TypeB();
	auto bb = 0;
	std::cout << typeid(ta).name() << std::endl;
	std::cout << typeid(tb).name() << std::endl;
	std::cout << typeid(x).name() << std::endl;
	std::cout << typeid(bb).name() << std::endl;
	//C++14����Ȃ��ƃR���p�C���ł��Ȃ�
#ifdef CPP14
	auto f = [](auto& t){ t.Display(); }; //test!!
	std::cout << typeid(f).name() << std::endl;
	f(ta); //auto�̃e�X�g
	f(tb); //auto�̃e�X�g
#endif

	//nullptr�̃e�X�g
	TypeA* pta = nullptr;
	pta->NoReferenceFunction(); //����͂ǂ̃P�[�X�ł��ʂ��I
	pta = NULL; //nullptr�Ƃ̔�r�� OK
	pta = 0; //nullptr�Ƃ̔�r�� OK
	if (pta == nullptr) {
		std::cout << "nullptr" << std::endl; //nullptr�œ��ꂵ���ق����悳��
	}

	//�߂�l��nullptr�̂��auto�Ŏ󂯂��nullprt_t�ɂȂ�BNULL�̏ꍇ��int�ɂȂ��Ă��܂���r���ł��Ȃ��Ȃ�B
	auto np = [](){ return nullptr; }; //�߂��NULL�ɂ���ƃG���[
	auto tp = np();
	std::cout << typeid(tp).name() << std::endl;
	if (tp == nullptr) { //NULL�ł�������ꍇ�͂����ł�����I
		std::cout << "nullptr 2" << std::endl;
	}


	//enum
	enum OldPhase {
		Invalid,
		Start,
		Processing,
		End
	};
	enum class Phase {
		Invalid,
		Start,
		Processing,
		End
	};
	OldPhase phase1 = Invalid;
	Phase phase2 = Phase::Start; //Class�����ق����݂₷���Ă����ˁI
	int phase3 = static_cast<int>(Phase::Invalid); //������int����Ȃ��̂�ˁc


	//�֐���delete
	//Increment c; //�R���X�g���N�^�̍폜�͂���܂�Ӗ��Ȃ����� C2880 �폜���ꂽ�֐� �� C2512 �R���X�g���N�^���Ȃ� �ɂȂ�
	Increment c(0);
	c.AddNumber(10);
	c.AddNumber(10.0); //cast����Ēʂ����Ⴄ���ǁI delete�͂���Ă���


	//MoveSemantics����炩���Ă݂�e�X�g
	std::cout << "Test Build 1" << std::endl;
	MoveClass::Relay<TypeA>(std::move(ta)); ta.Display(); //main�X�R�[�v�̕ϐ��Ɠ����A�h���X�ɂȂ�(Factory/Builder�p�^�[���ł͂܂������ǁA�ʏ탁�\�b�h�ł͑������������҂ł���)
	std::cout << "Test Build 2" << std::endl;
	auto&& ta2 = MoveClass::Build<TypeA>(); ta2.Display(); //Build�̒��Œ�`�������̓����A�h���X�����A���[�J���ϐ��Ȃ̂ŉ���
	std::cout << "Test Build 3" << std::endl;
	auto ta3 = MoveClass::Build<TypeA>(); ta3.Display();  //�ʃI�u�W�F�N�g(�R�s�[)�ƂȂ�

	//Perfect Transfar
	auto ctrf = [](int&& a, const int b) {  std::cout << (a + b) << std::endl; };
#ifndef CPP14
		Transpoter<decltype(ctrf)> trp = Transpoter<decltype(ctrf)>(ctrf); //C++14����C3497����������
#endif
	auto trp = Transpoter<std::function<void(int&&, const int)>>(ctrf);
	trp.Transfar(10, 20);
	//int aa = 5; trp.Transfar(aa, c.GetNumber()); //�R���p�C������error�ɂ��Ă�����I ���S�]���I

	std::cin >> x;
}