# Examples

## [01.SVG](../Examples/01.SVG/):

This is the most basic example of QRMatrix. It shows how to encode some sample texts into QR Code and write out to SVG files.

> Please edit path at `OUTPUT_PREFIX` in [main.cpp](../Examples/01.SVG/main.cpp) to store the output SVG files.

- Language: C++.
- Tools: CMake 3.27, QT Creator 8.0.1.
- Test platforms: Fedora 37; Mac OS X 12.7.

## [02.LibSPNG](../Examples/02.LibSPNG/):

This example shows how to make a QR Code PNG file.

This example uses `libspng` installed in my system:
- On Fedora, I installed `libspng` (including `devel` package) from Fedora repository.
- On MacOS I use `HomeBrew` to install it.
- Please check [CMakeLists.txt](../Examples/02.LibSPNG/QRMatrixExample/CMakeLists.txt) to configure on your case.

> Please edit path at `OUTPUT_PREFIX` in [main.cpp](../Examples/02.LibSPNG/QRMatrixExample/main.cpp) to store the output PNG files.

- Language: C++/C.
- Tools: CMake 3.27, QT Creator 8.0.1.
- Test platforms: Fedora 37; Mac OS X 12.7.

## [03.QT](../Examples/03.QT/):

This example shows how to make a `QPixmap` QR Code and display it on a window.

And of course, this example requires QT SDK.

- Language: C++.
- Tools: CMake 3.27, QT Creator 8.0.1.
- Test platforms: Fedora 37; Mac OS X 12.7.

## [04.CoreGraphic](../Examples/04.CoreGraphic/):

This example shows how to make a `UIImage/NSImage` QR Code in **iOS/MacOS** XCode app project.

It's the same way to use C++ source code in iOS and MacOS project. We need create a XCode project, import all C++ source code into it. We create a wrapper class in ObjC, rename file extension from `.m` to `.mm`, then we can use C++ functions in the wrapper class.

If the project is in Swift, we have to create `bridging.h` header file to allow Swift code using ObjC functions. More simple, we can make a separate framework project for C++ and wrapper ObjC class. Now the main project sees it as a module and we does not feel the difference of languages. (In this example, please use `QRMatrixExample` project to start, `QRMatrix` is the wrapper framework project).

We can use C++ source code with Swift too, but it requires latest XCode and Swift version.

In this example, I have 2 separated folders for iOS and MacOS. But you will find the codes to draw QR Code `UIImage/NSImage` almost the same (just a little different the step to create `UIImage/NSImage` object).

iOS/MacOS SDK (`ObjC NSData; NSString/Swift Data; String`) supports required String Encoding (Charset) for QR Code, so we don't need source code in [String folder](../String/) to convert text. But I still import them to do example about making QR Segments from [UnicodePoint class](../String/unicodepoint.h).

- Language: C++/ObjC (ObjCpp)/Swift).
- Tools: XCode 14.
- Test platforms: Mac OS X 12.7, iOS.

## [05.Android](../Examples/05.Android/):

This example shows how to make a `Bitmap` (via `Canvas`) QR Code in Android project.

You need to configure your project to support NDK. So Kotlin source code can call C++ functions via JNI.

This process is a bit complex. The communication via JNI almost uses binary data. So we have to convert our input data (text) to data type supported by JNI (`jstring`, `jbytearray` ...). Next step is convert j-type datas to C++ data. And when we return the result, we have to do similar process.

In this example, because Kotlin String `toByteArray` does not support ShiftJIS encoding (and it never fails - it replaces the unable-to-convert characters to `?`), I use code in [String folder](../String/) to convert UTF-8 input string to other encoding string to use in QR Code.

- Language: C++/Kotlin.
- Tools: Android Studio Giraffe.
