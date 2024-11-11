# Guayadeque Music Player

Guayadeque is a lightweight and easy-to-use music player and music collection organizer
that can easily manage large music collections and supports smart playlists.
In the technical side, it's written in C++, uses the wxWidget toolkit and the
Gstreamer media framework.

Tiago T Barrionuevo [<thothix@protonmail.com>](mailto:thothix@protonmail.com)  
see [LICENSE](LICENSE)

- [Github](https://github.com/thothix/guayadeque)
- [Latest release](https://github.com/thothix/guayadeque/releases/latest)

---

# French local and national radios advice

Guayadeque updates its radios database from the Tunein database. However all the radio stations of Radio France
(France Inter, France Musique, France Culture, France Info, F.I.P, France Bleue) are no more in the Tunein database.
There seems to be some conditions to be referenced by tunein that are not admissible for national radios such as:

    allowing tunein to add advertising when it's used to listen to them
    allowing tunein to infringe the copyrights, music rights...

Radio France has not found an agreement with Tunein and since april 2024 all its stations are no more referenced.
This is explained here, in French https://mediateur.radiofrance.com/actualites/lecoute-de-radio-france-via-tunein/

There's nothing to do in Guayadeque side to correct this issue: You need to manually add these Radio Stations if you
want to hear them through Guayadeque.

## List of the RadioFrance stations provided by icecast:

France Musique

    France Musique https://icecast.radiofrance.fr/francemusique-hifi.aac?id=radiofrance
    France Musique baroque https://icecast.radiofrance.fr/francemusiquebaroque-hifi.aac?id=radiofrance
    France Musique classique plus https://icecast.radiofrance.fr/francemusiqueclassiqueplus-hifi.aac?id=radiofrance
    France Musique concert Radio France https://icecast.radiofrance.fr/francemusiqueconcertsradiofrance-hifi.aac?id=radiofrance
    France Musique easy classique https://icecast.radiofrance.fr/francemusiqueeasyclassique-hifi.aac?id=radiofrance
    France Musique labo https://icecast.radiofrance.fr/francemusiquelabo-hifi.aac?id=radiofrance
    France Musique contemporaine https://icecast.radiofrance.fr/francemusiquelacontemporaine-hifi.aac?id=radiofrance
    France Musique jazz https://icecast.radiofrance.fr/francemusiquelajazz-hifi.aac?id=radiofrance
    France Musique Ocora musiques du monde https://icecast.radiofrance.fr/francemusiqueocoramonde-hifi.aac?id=radiofrance

FIP (France Inter Paris)

    FIP https://icecast.radiofrance.fr/fip-hifi.aac?id=radiofrance
    FIP electro https://icecast.radiofrance.fr/fipelectro-hifi.aac?id=radiofrance
    FIP groove https://icecast.radiofrance.fr/fipgroove-hifi.aac?id=radiofrance
    FIP hiphop https://icecast.radiofrance.fr/fiphiphop-hifi.aac?id=radiofrance
    FIP jazz https://icecast.radiofrance.fr/fipjazz-hifi.aac?id=radiofrance
    FIP metal https://icecast.radiofrance.fr/fipmetal-hifi.aac?id=radiofrance
    FIP nouveautés https://icecast.radiofrance.fr/fipnouveautes-hifi.aac?id=radiofrance
    FIP pop https://icecast.radiofrance.fr/fippop-hifi.aac?id=radiofrance
    FIP reggae https://icecast.radiofrance.fr/fipreggae-hifi.aac?id=radiofrance
    FIP rock https://icecast.radiofrance.fr/fiprock-hifi.aac?id=radiofrance
    FIP sacré français https://icecast.radiofrance.fr/fipsacrefrancais-hifi.aac?id=radiofrance
    FIP world https://icecast.radiofrance.fr/fipworld-hifi.aac?id=radiofrance

France Culture https://icecast.radiofrance.fr/franceculture-hifi.aac?id=radiofrance

France Inter https://icecast.radiofrance.fr/franceinter-hifi.aac?id=radiofrance

France Info http://icecast.radiofrance.fr/franceinfo-hifi.aac
