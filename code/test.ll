; ModuleID = 'eokas'
source_filename = "eokas"

%Type.String.Struct = type { i8* }

declare i32 @puts(i8*)

declare i32 @printf(i8*, ...)

declare i32 @sprintf(i8*, i8*, ...)

declare i8* @malloc(i64)

declare void @free(i8*)

define i32 @print(%Type.String.Struct* %0) {
entry:
  %1 = getelementptr inbounds %Type.String.Struct, %Type.String.Struct* %0, i32 0, i32 0
  %2 = load i8*, i8** %1, align 8
  %3 = call i32 @puts(i8* %2)
  ret i32 %3
}

define i32 @main() {
entry:
  %foo = alloca i32 (i32)*, align 8
  store i32 (i32)* @0, i32 (i32)** %foo, align 8
  %0 = load i32 (i32)*, i32 (i32)** %foo, align 8
  %1 = call i32 %0(i32 2)
  ret i32 0
}

define i32 @0(i32 %n) {
entry:
  br label %if.begin

if.begin:                                         ; preds = %entry
  %0 = icmp sle i32 %n, 0
  br i1 %0, label %if.true, label %if.false

if.true:                                          ; preds = %if.begin
  ret i32 1
  br label %if.end

if.false:                                         ; preds = %if.begin
  %2 = sub i32 %n, 1
  %3 = call i32 @0(i32 %2)
  %4 = mul i32 %n, %3
  ret i32 %4
  br label %if.end

if.end:                                           ; preds = %if.false, %if.true
  ret i32 0
}