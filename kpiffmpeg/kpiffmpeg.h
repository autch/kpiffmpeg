// 以下の ifdef ブロックは DLL からのエクスポートを容易にするマクロを作成するための 
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された KPIFFMPEG_EXPORTS
// シンボルを使用してコンパイルされます。このシンボルは、この DLL を使用するプロジェクトでは定義できません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、 
// KPIFFMPEG_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef KPIFFMPEG_EXPORTS
#define KPIFFMPEG_API __declspec(dllexport)
#else
#define KPIFFMPEG_API __declspec(dllimport)
#endif

extern "C" KMPMODULE* APIENTRY kmp_GetTestModule();
extern "C" BOOL WINAPI kmp_GetTestTagInfo(const char *cszFileName, IKmpTagInfo *pTagInfo);
