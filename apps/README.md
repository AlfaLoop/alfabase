Alfabase �p�{��
================

## ����O Alfabase �p�{��
#### �p�G�� Alfabase �˸m��릨�@�ӤH���ܡA����p�{���N�O�F��AAlfabase �N�O���g�A�w��˸m���ߡC  
�b�o���|�@��²�檺�Ҥl�����AAndroid ����ѦU�a�t�Өӻs�@�B��}�ۮa�� OS �����A�åB�U�a������j�h���i�H�b Google play �ө����U��������ε{���ϥΡC
�o��Ҵ��쪺������ε{������p�{���������O�@�˪��A����N��f�� Alfabase OS ���˸m�O�P�˷����C  
�b�U�ӵw��˸m����¦���W(�Ҧp�Y�����B�Y beacon ����...)�A�}�o�̦b�}�o���@�ظ˸m���Ϊ��ɭԡA
�ݭn�q�̩��h�}�l�A�X�ʡB���ε��������i���V�W�h���ε{���}�o�A�z�L Alfabase OS ���p�{���h���ݭn�`�J���h�]��i��W�h���ζ}�o�A
�Ʀܤ��ݭn�Ҽ{��ٹq�����A�åB�P�ɶ��i�H���� Alfabase OS ���޲z�P�O�@�A�h�Ӥp�{�����w�ˡB�����γ]�w�����A���}�o�U�ظ˸m�ܱo��²��C  
  
  
## �p�{���}�o�y�{
<p align="center">
<img src="./images/work_flow.png"><br/>
�Ϥ@�B�}�o�y�{��
</p>  

## ELF loader
ELF loader �O Contiki OS ���@�����O�J���t�Υi�H�b���ݭn���m�έ��Ҹ˸m�����p�U�N����H�ɩ⴫�֤ߡu���ε{���v���Ҷ��C
Alfabase ��X�F Contiki OS�A�}�o�H������b Alfabase �����ϥγo�ӼҶ��åB�z�L�I�s [Alfabase API](https://www.alfaloop.com/docs/alfaos/#os-framework) �}�o�uAlfabase �p�{���v�C
  
## Alfabase �p�{���}�o�u��
�}�o Alfabase �p�{�����F [Alfabase](https://github.com/AlfaLoop/alfabase) �H�~�A�ٻݭn [alfabridge](https://github.com/AlfaLoop/alfabridge-android) ���u��A�Ʀܥi�H�f�t [nRF Connect for Mobile](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) �����P���աC
  
- [Alfabase](https://github.com/AlfaLoop/alfabase)  
Alfabase �t�ΡA�ؿ����� [tools/](https://github.com/AlfaLoop/alfabase/tree/master/tools) ��Ƨ��� [alfa.py](https://github.com/AlfaLoop/alfabase/blob/master/tools/alfa.py) �u��ΨӺ޲z�p�{���M�סA�i�غc�B�sĶ�ΤW�Ǥp�{���� Alfabase �˸m���C
�b [apps/](https://github.com/AlfaLoop/alfabase/tree/master/apps) ��Ƨ������@�ǽd�Ҥp�{���i�H�ѦҡA�Ҧp [HelloWorld](https://github.com/AlfaLoop/alfabase/tree/master/apps/helloworld)�C
  
- [alfabridge](https://github.com/AlfaLoop/alfabridge-android)  
alfabridge �ثe��� Android �t�ΡA�åB�Ȭۮe�����Ť� 4.0(BLE) �Ҷ����˸m�C�D�n�ΨӨ�U PC �P�˸m�����p�{���ǿ�P���p�C
  
## �p��}�o
- �إ߱M��  
�b apps/ ���I�s [alfa.py](https://github.com/AlfaLoop/alfabase/blob/master/tools/alfa.py) �إ߷s�p�{���M��  
```bash
cd alfabase/apps/
python ../tools/alfa.py new
```
�åB��J�M�צW��(�p�{���W��)�P�}�o�̦W��(�D����)  
<p align="center">
<img src="./images/new_proj.png"><br/>
�ϤG
</p>  
  
- �}�o�P�sĶ  
�|���ͭ��إߪ��p�{���M�׸�Ƨ��A�i�J��|���򥻪� app.conf �P main.c �ɮסC
��Ƨ����� app.conf �y�z�p�{�����M�׳]�w�Amain.c �h�O�p�{�����D�n�{�ǡA��l�ɥ]�t�򥻪� Timer �M Logger ���ϥνd�ҡC  
�p�{�����N�X�}�o�i�H�Ѧ� [apps/](https://github.com/AlfaLoop/alfabase/tree/master/apps) ��L�d�ұM�׻P [Alfabase API](https://www.alfaloop.com/docs/alfaos/#os-framework)�C
���U�Ӷi��p�{�����sĶ
```bash
cd PROJECT-NAME
python ../../tools/alfa.py build
```
<p align="center">
<img src="./images/build_app.png"><br/>
�ϤT
</p>  
���ͪ� PROJECT-NAME-app.elf �N�O�p�{���sĶ�ɡC
  
- �W�ǻP����  
���ͤF .elf �ɫ�A�N�i�H�W�Ǩ�˸m������A�ݭn�f�t alfabridge �W�Ǥp�{���C  
�������إ� alfabridge �P�˸m���s�u�A  
<p align="center">
<img src="./images/start.jpg"><br/>
�ϥ|
</p>  
�I���}�l�]�w�������� Alfabase �˸m�A  
  
<p align="center">
<img src="./images/discovering.jpg"><br/>
�Ϥ�
</p>  
  
�a�� Alfabase �˸m�@�q�ɶ��A  
<p align="center">
<img src="./images/connedted.jpg"><br/>
�Ϥ��B�P�˸m�����t��  
</p>  
  
��������� Alfabase �˸m��Aalfabridge �|�M�˸m�s�u�A�ڭ̦b�z�L alfabridge �̹��W���ϰ���� ip ��}�� Alfabase �˸m�U�R�O�Y�i��˸m�i��ާ@�C  
alfa.py �� install �P run �R�O���O�N��W�Ǥp�{���ΰ���p�{���A���n���[�Ѽ� -i IP_ADDRESS (alfabridge �W��ܪ� ip ��})�C  
```bash
python ../../tools/alfa.py install -i 192.168.1.XX
python ../../tools/alfa.py run -i 192.168.1.XX
```
<p align="center">
<img src="./images/deploy.png"><br/>
�ϤC  
</p>  
�z�L�H�W�R�O�Y�i�����p�{�������p�I  
  
PS.
1. Android �˸m�����P PC �b�P�@�Ӱϰ�������C  
2. �n��˸m�U�F run�Bkill�Bsetboot�Bdelboot �����R�O�A���b�����ݪ��p�{���M�ץظ��U�B�� alfa.py  
  
## alfa.py �u��R�O�ﶵ
  
```bash
python alfa.py �R�O [-i IP_ADDRESS]
```
  
|�R�O|�y�z�B����|
|:---:|:---:|
|new|�إߤp�{���M��|
|build|�sĶ�p�{�� elf ��|
|clean|�M���M�פ����sĶ��|
|install|�W�ǫ��w�M�פp�{����˸m|
|remove|�����˸m�����p�{��(���w�M��)|
|remove_app_files|�����˸m�����Ҧ��p�{��|
|run|������w�M�פp�{��|
|kill|������椤���p�{��|
|ts|�P�B�ɶ���˸m��|
|list|��Ū�˸m���w�w�˪��p�{���C��|
|setboot|�]�m���椤���p�{�����˸m�}���۰ʰ���|
|delboot|�M���˸m�}���۰ʰ��檺�p�{���]�m|
|airlog|��ť�˸m�� air log �T��|
|version|���L��e����
  
PS. PC ���a�ݾާ@���R�O�L�ݪ��[ -i �ѼơA�p new�Bbuild�Bclean�Bversion ���C
  
  
## Alfabase �˸m���A

Alfabase �˸m�b���U�F����R�O���ɭԡA�ݩ�L���A�C  
�U�F run �R�O����A�˸m�|�}�l������w���p�{���A�Ҧp footmotion �c�����ΡA�o�ɸ˸m�����A�����椤�C
�b���椤�����A�U�A�L�k�A���U�F run ���R�O�A���O�i�H�z�L kill �ϸ˸m�^��L���A�Akill �R�O�|�פ��e���椤���p�{���C  
setboot �R�O�|�]�m��e���椤���p�{�����˸m�}���۰ʰ���A��˸m�󴫹q���έ��s�Ұʤ���A�]�m�۰ʱҰʪ��p�{���|�b�}����۰ʰ���A
setboot �����b���椤�����A�U�~�i�ϥΡCdelboot �h�Ϥ��A���n�w�]�m setboot �����A�U�~�i�ϥΡC
  
<p align="center">
<img src="./images/state_graph.png"><br/>
�ϤK�B�˸m���A�ഫ��  
</p>  
















