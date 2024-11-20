#include <iostream>
#include <bitset>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <ctime>

using namespace std;

// Constants for UART Simulation
const int START_BIT = 0;
const int STOP_BIT = 1;
const int DATA_BITS_DEFAULT = 8;  // Default 8 data bits
int DATA_BITS = DATA_BITS_DEFAULT;  //frame size customization
const int BAUD_RATE = 9600; 
const double BIT_DURATION = 1.0 / BAUD_RATE;  


bool receiver_ready = true;  


int calculate_parity(int data) {
    int parity = 0;
    for (int i = 0; i < DATA_BITS; ++i) {
        parity ^= (data >> i) & 1;
    }
    return parity;
}


vector<int> uart_tx(int data, bool parity_enabled, bool even_parity, bool introduce_errors, double error_probability) {
    
    while (!receiver_ready) {
        cout << "Receiver not ready, waiting..." << endl;
        this_thread::sleep_for(chrono::milliseconds(50));  // Wait before retrying
    }


    vector<int> tx_data = {START_BIT};


    for (int i = 0; i < DATA_BITS; ++i) {
        int bit = (data >> i) & 1;
        tx_data.push_back(bit);
    }


    if (parity_enabled) {
        int parity = calculate_parity(data);
        if (!even_parity) {
            parity = !parity;
        }
        tx_data.push_back(parity);
    }


    tx_data.push_back(STOP_BIT);


    for (size_t i = 0; i < tx_data.size(); ++i) {
        int transmitted_bit = tx_data[i];

        if (introduce_errors && ((rand() / double(RAND_MAX)) < error_probability)) {
            transmitted_bit ^= 1;  
        }

        cout << "TX Bit: " << transmitted_bit << endl;
        this_thread::sleep_for(chrono::duration<double>(BIT_DURATION));  
    }

    receiver_ready = true;
    return tx_data;
}


int uart_rx(const vector<int>& tx_data, bool parity_enabled, bool even_parity) {
    
    receiver_ready = false; 

    vector<int> received_data;

    for (int bit : tx_data) {
        received_data.push_back(bit);
        this_thread::sleep_for(chrono::duration<double>(BIT_DURATION));  
    }

    received_data = vector<int>(received_data.begin() + 1, received_data.end() - 1);  

    int received_parity = -1;
    if (parity_enabled) {
        received_parity = received_data.back();  
        received_data.pop_back();  
    }

    int received_value = 0;
    for (int i = 0; i < DATA_BITS; ++i) {
        received_value |= (received_data[i] << i);  
    }

    if (received_data.back() != STOP_BIT) {
        cout << "Framing error detected! Incorrect stop bit." << endl;
    }

    if (parity_enabled) {
        int calculated_parity = calculate_parity(received_value);
        if (even_parity) {
            if (calculated_parity != received_parity) {
                cout << "Parity error detected!" << endl;
            }
        } else {
            if (calculated_parity == received_parity) {
                cout << "Parity error detected!" << endl;
            }
        }
    }

    auto receive_time = chrono::high_resolution_clock::now();

    vector<bool> bits(DATA_BITS);
    for (int i = 0; i < DATA_BITS; ++i) {
        bits[i] = (received_value >> i) & 1;
    }

    cout << "Received data: " << received_value << " (0b";
    for (int i = DATA_BITS - 1; i >= 0; --i) {
        cout << bits[i];
    }
    cout << ")" << endl;

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end_time - receive_time;
    cout << "Reception took " << duration.count() << " seconds." << endl;

    receiver_ready = true; 
    return received_value;
}

void simulate_uart() {
    bool parity_enabled = true;     
    bool even_parity = true;         
    bool introduce_errors = true;    
    double error_probability = 0.05; 
    int num_bytes = 5; 

    for (int i = 0; i < num_bytes; ++i) {
        int data_to_send = rand() % 256;  
        cout << "\nTransmitting byte #" << (i + 1) << " (data: " << data_to_send << ")\n";
        
        vector<int> transmitted_data = uart_tx(data_to_send, parity_enabled, even_parity, introduce_errors, error_probability);
        
        int received_data = uart_rx(transmitted_data, parity_enabled, even_parity);
    }
}

int main() {
    srand(time(0));  //seeding for rand()
    simulate_uart();
    return 0;
}
