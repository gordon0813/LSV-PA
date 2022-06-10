cd /Users/gordon/Documents/Lab/oss-cad-suite
./script.sh
cd /Users/gordon/Documents/GitHub/LSV-PA
./abc -f ./script_test.sh
dot -Tpng test.dot > output.png
