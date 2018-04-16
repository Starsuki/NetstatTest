# **关于 NetstatTest v1.0**
## 1.**简单介绍**
这是我在尝试对32位win10下的 **netstat.exe** 进行逆向分析后，根据其原理实现了 **Netstat** 中 **-a** **-n** **-o** 及 **interval** 功能。

当然，它与真正的 Netstat 相差甚远。首先，我只实现了四个参数的功能(当然它们组合起来的功能我是实现了的)；其次，显示结果中只包含了对 TCP4Table和UDP4Table 的分析，未处理 TCP6Table 以及 UDP6Table ，而且为了简化 “netstat interval” 功能实现的过程，我将间隔限制在了0-3s。

在实现过程中，有些地方采用了和原生 Netstat 不一样的实现方式。比如OD对代码的分析结果显示， Netstat 调用了 **InternalGetTcpTable2** ，而这个API相关的文档难以寻找，于是我使用了 **GetTcpTable2** 作为替代。还有一点， Netstat 中消息表的实现方式我没能分析出来，于是自己查资料用 **mc.exe** 实现了一个。

代码中加了少的可怜的注释，希望过一段时间我还能看懂它吧...
## **2.参考资料**
程序实现过程中的大部分文档来自 msdn ,还参考了一些技术博客及论坛。
## **3.实现关键**
程序实现的关键之处是对 TCPtable 和 UDPtable 的读取和分析。相关API的用法很容易找到。

另外，为了尽可能还原原生 Netstat 的实现，我没有采用简单的将显示结果一点点 printf 的方式，而是构建的一个消息资源表，使用 **FormatMessage** 产生消息，再使用 **stdout** 文件流的方式将结果输出到控制台中。这个过程涉及到 **Message Compiler** 的使用，以及可变参数函数的实现，相关文档不容易找到，因此将这个过程总结在这里。
### **3.1 相关文档**
关于 **mc.exe** 的用法和 **.mc** 文件的撰写，可以参考以下三个文档。

**Message Compiler** 的用法：[**Message Compiler**](https://msdn.microsoft.com/zh-cn/library/windows/desktop/aa385638(v=vs.85).aspx)

**.mc** 文件的格式：[**Message Text Files**](https://msdn.microsoft.com/zh-cn/library/windows/desktop/dd996906(v=vs.85).aspx)，[**FormatMessage**](https://msdn.microsoft.com/zh-cn/library/windows/desktop/ms679351(v=vs.85).aspx)

一个 **.mc** 文件的例子：[**Sample.mc**](https://msdn.microsoft.com/en-us/library/windows/desktop/bb540472(v=vs.85).aspx)
### **3.2  .mc 文件的处理**
在写好 **Message .mc** 文件后，我们需要使用 **mc.exe** 对其进行处理。

首先在 cmd 中运行文件 **C:\Program Files\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build>vcvars32** 。只有这样才可以使用 mc 命令。

之后在 cmd 中打开 **Message .mc** 所在目录，输入 **mc -a -A Message .mc**。**mc.exe** 在 **Message .mc** 所在目录下生成四个文件： **Message.h** 、 **Message.rc** 、 **MSG00409.bin** 、 **MSG00804.bin**。其中， **Message.h** 需被添加到工程头文件目录中，并include； **Message.rc** 需被添加到工程资源文件中；剩下两个文件与消息表中语言有关，这里分别是中文和英文。

这样我们就可以结合 **FormatMessage** 使用自己构建的消息表了。

### **3.3 在函数中使用可变参数**
**FormatMessage** 通过传入一个 **va\_list** 指针来完成对各个待拼接字符串的引入，而 **va\_list** 中参数的数目是不一定的，因为组成消息的字符串的数目是不一定的。所以需要在函数中使用可变参数获取数目不定的参数。

函数 **getFormattedMessage** 的定义： **LPWSTR getFormattedMessageW(int noNewLine, int messageId, ...){...（省略函数内部代码）}** 。形参列表最后的“...”表示可变参数。经实验，如果调用该函数时不需传入可变参数，须在该位置传入 **NULL** ，不能直接省略该参数。

再看看 **getFormattedMessage** 内的部分代码：
```
...

va_list args;
va_start(args, messageId);

...

if (FormatMessageW(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK,
	NULL,
	messageId,
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	(LPWSTR)&pMessBuf,
	0,
	&args))
{
	va_end(args);
	return pMessBuf;
}

...
```

首先定义一个 **va\_list** 类型 **args** 以接收参数。然后使用函数 **va\_start** 开始接收参数，此函数第一个参数是定义好的 **va\_list** 类型，第二个参数 **必须** 传入 **getFormattedMessageW** 形参中除可变参数的最后一个参数（紧挨着可变参数的参数），在这里就是 **messageId** 。在调用 **FormatMessage** 后，才能使用 **va\_end** 函数结束参数的接收。

在函数 **my_vsnwprintf** 中也使用了可变参数：
```
wchar_t * my_vsnwprintf(size_t size, wchar_t *format, ...)
{
	...

	va_list args;
	va_start(args, format);
	int dwRetVal = _vsnwprintf(buffer, size, format, args);
	va_end(args);

	...
}
```


