#include <iostream>
#include <functional> //std::function

//VSで検知できなかったので･･････
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
	void AddNumber(float) = delete; //deleteのテスト！
	int GetNumber(){
		return number;
	}
};
//C++14？
//template<> void Increment::AddNumber<bool>(bool) = delete;

//overrideはスキップ

//MoveSemantics
class MoveClass {
public :
	//受け取ったものとアドレスが変わらないものがかえる(moveのmove)
	//universalである必要がある
	template<typename T>
	static void Relay(T&& val) {
		val.Update(); //これは反映されない
		val = std::forward<T>(T());
	};

	//ローカル参照のアドレスと一致したものが帰る(ただし参照は壊れる！)
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
	//パラメータの型推論テスト
	int x = 27;
	TypeA ta;
	TypeB&& tb = TypeB();
	auto bb = 0;
	std::cout << typeid(ta).name() << std::endl;
	std::cout << typeid(tb).name() << std::endl;
	std::cout << typeid(x).name() << std::endl;
	std::cout << typeid(bb).name() << std::endl;
	//C++14じゃないとコンパイルできない
#ifdef CPP14
	auto f = [](auto& t){ t.Display(); }; //test!!
	std::cout << typeid(f).name() << std::endl;
	f(ta); //autoのテスト
	f(tb); //autoのテスト
#endif

	//nullptrのテスト
	TypeA* pta = nullptr;
	pta->NoReferenceFunction(); //これはどのケースでも通るよ！
	pta = NULL; //nullptrとの比較→ OK
	pta = 0; //nullptrとの比較→ OK
	if (pta == nullptr) {
		std::cout << "nullptr" << std::endl; //nullptrで統一したほうがよさげ
	}

	//戻り値がnullptrのやつはautoで受けるとnullprt_tになる。NULLの場合はintになってうまく比較ができなくなる。
	auto np = [](){ return nullptr; }; //戻りをNULLにするとエラー
	auto tp = np();
	std::cout << typeid(tp).name() << std::endl;
	if (tp == nullptr) { //NULLでもらった場合はここでこける！
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
	Phase phase2 = Phase::Start; //Classつけたほうがみやすくていいね！
	int phase3 = static_cast<int>(Phase::Invalid); //ただしintじゃないのよね…


	//関数のdelete
	//Increment c; //コンストラクタの削除はあんまり意味なさそう C2880 削除された関数 → C2512 コンストラクタがない になる
	Increment c(0);
	c.AddNumber(10);
	c.AddNumber(10.0); //castされて通っちゃうけど！ deleteはされている


	//MoveSemanticsをやらかしてみるテスト
	std::cout << "Test Build 1" << std::endl;
	MoveClass::Relay<TypeA>(std::move(ta)); ta.Display(); //mainスコープの変数と同じアドレスになる(Factory/Builderパターンではまずいけど、通常メソッドでは早い実装が期待できる)
	std::cout << "Test Build 2" << std::endl;
	auto&& ta2 = MoveClass::Build<TypeA>(); ta2.Display(); //Buildの中で定義したもの同じアドレスだが、ローカル変数なので壊れる
	std::cout << "Test Build 3" << std::endl;
	auto ta3 = MoveClass::Build<TypeA>(); ta3.Display();  //別オブジェクト(コピー)となる

	//Perfect Transfar
	auto ctrf = [](int&& a, const int b) {  std::cout << (a + b) << std::endl; };
#ifndef CPP14
		Transpoter<decltype(ctrf)> trp = Transpoter<decltype(ctrf)>(ctrf); //C++14だとC3497が発生する
#endif
	auto trp = Transpoter<std::function<void(int&&, const int)>>(ctrf);
	trp.Transfar(10, 20);
	//int aa = 5; trp.Transfar(aa, c.GetNumber()); //コンパイル時にerrorにしてくれるよ！ 完全転送！

	std::cin >> x;
}