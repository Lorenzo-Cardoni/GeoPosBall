clear all
clc

% Creazione oggetto TCP client
esp32 = tcpclient("192.168.4.1", 80);

while true
    % Ricezione dati
    data = read(esp32);
    
    % Visualizzazione dati
    % disp(char(data));
    
    % Elaborazione dati
    dati = str2double(strsplit(char(data), ','));
    
    % Assicurati che la dimensione del vettore sia corretta
    if length(dati) == 8
        time = dati(1);
        PDOP = dati(2);
        lon = dati(3);
        lat = dati(4);
        q0 = dati(5);
        q1 = dati(6);
        q2 = dati(7);
        q3 = dati(8);

        % Puoi ora elaborare i dati come desiderato
        disp(['Time: ', num2str(time),'s ', ' PDOP: ', num2str(PDOP), ' Lon: ', num2str(lon), ' Lat:' , num2str(lat) , ' q0: ', num2str(q0), ' q1: ', num2str(q1), ' q2: ', num2str(q2), ' q3: ', num2str(q3)]);
    else
        disp(char(data));
    end
end
