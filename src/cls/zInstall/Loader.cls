/// Loaderインストーラ
Class zInstall.Loader Extends %RegisteredObject
{

/// Loader.exeを実行させるためのCallinサービスの有効化
ClassMethod EnableCallin() As %Status
{
	set ret=##class(Security.System).Get("SYSTEM",.prop)
	quit:$$$ISERR(ret) ret
	set prop("AutheEnabled")=2047
	set ret=##class(Security.System).Modify("SYSTEM",.prop)
	quit:$$$ISERR(ret) ret
	
	set p("Enabled")=1
	set p("AutheEnabled")=48
	set ret = ##class(Security.Services).Modify("%Service_CallIn",.p)
	quit ret
}

}
