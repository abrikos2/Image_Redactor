# Image_Redactor
Для запуска GUI из docker требуется установленная программа VcXsrv или любой другой X server(разберу с VcXsrv)
1.Выбрать Multiple Windows, Strart no client, поставить галочку на пункте disable acess control
2.Собрать docker образ
3.Для запуска использовать команду docker run --rm -it -e DISPLAY=192.168.0.100:0.0 image_redactor:latest или docker run --rm -it -e DISPLAY=host.docker.internal:0.0 image_redactor:latest
На Windows откроется окно программы.
Для выбора изображения требуется поместить его в docker container командой docker cp C:\Путь к изображения на Windows mystifying_khayyam:/Путь к изображения в docker
