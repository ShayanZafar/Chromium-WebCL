for f in `ls *.h *.cpp`
do
	cat $f | sed 's/ExceptionCode&/ExceptionState\&/g' | sed 's/ExceptionCode &/ExceptionState\&/g' | sed 's/ec = \(.*\);/ec.throwDOMException(\1);/g' | sed 's/ExceptionCode\.h/..\/..\/bindings\/v8\/ExceptionState.h/' > $f.tmp;
	mv $f.tmp $f
done
