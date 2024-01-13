import pandas as pd
from pyproj import Proj
import os.path

zone_number = 33

def convert_dd_to_utm(latitude, longitude):
    # Definisci la proiezione UTM
    utm_proj = Proj(proj='utm', zone=zone_number, ellps='WGS84')

    # Effettua la conversione delle coordinate
    utm_easting, utm_northing = utm_proj(longitude, latitude)

    return utm_easting, utm_northing

def dm_to_dd(degrees, minutes):
    dd = float(degrees) + float(minutes) / 60
    return dd

def trasforma_numero(numero):
    # Converte il numero in una stringa

    numero_str = str(numero)

    # Estrae i primi due caratteri come parte prima
    parte_prima = numero_str[:2]

    # Estrae il resto della stringa come parte dopo
    parte_dopo = numero_str[2:]

    # Unisce la parte prima con uno spazio e la parte dopo con un punto decimale
    risultato = parte_prima + ' ' + parte_dopo[:2] + '.' + parte_dopo[2:]

    # Restituisce il risultato
    return risultato.split(' ')


def update_coordinate(val):
  vec = trasforma_numero(val)
  return dm_to_dd(vec[0],vec[1])


input_filename = "Dati_misa_05122023.csv"
output_filename = "converted_misa_05122023.csv"



def convert_csv(input_filename, output_filename):
    df = pd.read_csv(input_filename)
    df2 =  df[["latitude_2","longitude_2","PDOP"]].copy()
    
    for i,x in enumerate(df2["latitude_2"]):
        if x == 0:
            continue
        vec = trasforma_numero(x)
        df2["latitude_2"].iloc[i] = dm_to_dd(vec[0],vec[1])

    for i,x in enumerate(df2["longitude_2"]):
        if x == 0:
            continue
        vec = trasforma_numero(x)
        df2["longitude_2"].iloc[i] = dm_to_dd(vec[0],vec[1])
    
    east = []
    north = []

    for lat,lon in zip(df2["latitude_2"], df2["longitude_2"]):
        x,y = convert_dd_to_utm(lat,lon)
        east.append(x)
        north.append(y)

    df2["utm_east"] = east
    df2["utm_north"] = north
    df2.head()

    df2.to_csv(output_filename)


convert_csv(input_filename,output_filename)

