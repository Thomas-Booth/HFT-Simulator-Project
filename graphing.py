import socket
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import threading
from threading import Lock
import time


class RealTimeGrapher:
    def __init__(self):
        
        # Data storage - Add a max_length to the deque's to limit data points plotted
        self.bid_prices = deque()
        self.ask_prices = deque()
        self.portfolio_values = deque()
        self.line_numbers = deque()
        
        # Thread synchronization
        self.data_lock = Lock()
        self.last_update_time = time.time()
        
        # Socket setup
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            self.sock.bind(('127.0.0.1', 8888))
            print("Successfully bound to port 8888")
        except Exception as e:
            print(f"Failed to bind to port 8888: {e}")
            return
            
        self.sock.settimeout(0.1)
        
        # Plot setup
        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(12, 8))
        self.fig.suptitle('Real-Time Trading Data - Waiting for data...')
        
        # Price lines
        self.line_bid, = self.ax1.plot([], [], 'g-', linewidth=1, label='Bid Price')
        self.line_ask, = self.ax1.plot([], [], 'r-', linewidth=1, label='Ask Price')
        
        #! UPDATE THESE HORIZONTAL LINES' VALUES IF USING SUPPORT/RESISTANCE STRATEGY
        # Static support/resistance lines (drawn once, not updated) - Used to show support/resistance levels
        self.ax1.axhline(y=1.35000, color='black', linestyle='--', linewidth=2, alpha=0.7, label='Resistance Value')
        self.ax1.axhline(y=1.28000, color='black', linestyle='--', linewidth=2, alpha=0.7, label='Support Value')
    
        
        self.ax1.set_title(f'Bid/Ask Prices')
        self.ax1.set_xlabel('Tick')
        self.ax1.set_ylabel('Price')
        
        #! UPDATE THE Y_LIMITS TO FIT THE GRAPH YOU WANT TO VIEW
        self.ax1.set_ylim(1.20, 1.45)
        
        
        self.ax1.legend()
        self.ax1.grid(True, alpha=0.3)
        
        # Portfolio line
        self.line_pv, = self.ax2.plot([], [], 'b-', linewidth=2, label='Portfolio Value')

        #! UPDATE THE HORIZONTAL LINE SHOWING STARTING BALANCE IF CHANGED IN MAIN.C
        # Static starting balance line
        self.ax2.axhline(y=10, color='orange', linestyle='--', linewidth=2, alpha=0.7, label='Starting Balance')


        self.ax2.set_title(f'Portfolio Value (in 100,000s)')
        self.ax2.set_xlabel('Tick')
        self.ax2.set_ylabel('Portfolio Value ($)')
        
        #! UPDATE Y_LIMIT VALUES TO FIT GRAPH YOU WANT TO VIEW
        self.ax2.set_ylim(9, 11)
        
        
        self.ax2.legend()
        self.ax2.grid(True, alpha=0.3)
        
        # Start data receiver thread
        self.running = True
        self.data_received_count = 0
        self.data_thread = threading.Thread(target=self.receive_data, daemon=True)
        self.data_thread.start()
        
        print("Python grapher started - waiting for C program data...")


    # Control the data coming into the script and assign to lists
    def receive_data(self):
        # Variables to check if values are missing
        consecutive_errors = 0
        max_consecutive_errors = 10
        
        while self.running:
            try:
                data, addr = self.sock.recvfrom(1024)
                data_str = data.decode('utf-8').strip()
                
                # Reset error counter on successful receive
                consecutive_errors = 0
                
                # Parse CSV format: line,bid,ask,portfolio
                parts = data_str.split(',')
                if len(parts) == 4:
                    try:
                        line_num = int(parts[0])
                        bid = float(parts[1])
                        ask = float(parts[2])
                        portfolio = float(parts[3])
                    except ValueError as e:
                        print(f"Parsing error: {e}")
                        continue  # skip this packet
                    
                    # Process each message with thread safety
                    with self.data_lock:
                        try:
                            # Add data atomically
                            self.line_numbers.append(line_num)
                            self.bid_prices.append(bid)
                            self.ask_prices.append(ask)
                            self.portfolio_values.append(portfolio)
                            
                            self.data_received_count += 1
                            self.last_update_time = time.time()
                            
                            # Debug output
                            #print(f"Data packets received: {self.data_received_count}")
                            
                        except (KeyError, ValueError, TypeError) as e:
                            print(f"Data processing error: {e}")
                
            except socket.timeout:
                # Check if we haven't received data for too long
                if time.time() - self.last_update_time > 10:  # 10 seconds
                    print("No data received for 10 seconds...")
                continue
                
            except Exception as e:
                consecutive_errors += 1
                print(f"Network error ({consecutive_errors}/{max_consecutive_errors}): {e}")
                
                if consecutive_errors >= max_consecutive_errors:
                    print("Too many consecutive errors, stopping data receiver")
                    break
                
                # Pause a little before retrying
                time.sleep(0.1)  

    # Downsample data so we aren't plotting too many in large files
    def downsample(self, x, y, max_points):
        n = len(y)
        if n <= max_points or max_points <= 0:
            return x, y
        step = n // max_points
        x_ds = x[::step]
        y_ds = y[::step]
        return x_ds, y_ds


    # Update data drawn on lines in the graph
    def animate(self, frame):
        try:
            # Thread-safe data access
            with self.data_lock:
                # Make local copies and ensure consistency
                data_count = self.data_received_count
                
                # Ensure all arrays have same length
                min_length = min(len(self.line_numbers), len(self.bid_prices), 
                           len(self.ask_prices), len(self.portfolio_values))
                
                if min_length == 0:
                    return self.line_bid, self.line_ask, self.line_pv
                
                # Create synchronized data arrays
                line_numbers_safe = list(self.line_numbers)[:min_length]
                bid_prices_safe = list(self.bid_prices)[:min_length]
                ask_prices_safe = list(self.ask_prices)[:min_length]
                portfolio_values_safe = list(self.portfolio_values)[:min_length]

            # Downsample the data - THIS CAN BE ALTERED TO BE MORE/LESS PRECISE    
            x_bid, y_bid = self.downsample(line_numbers_safe, bid_prices_safe, 5000)
            x_ask, y_ask = self.downsample(line_numbers_safe, ask_prices_safe, 5000)
            x_pv, y_pv = self.downsample(line_numbers_safe, portfolio_values_safe, 2000)
            
            # Updating line data
            self.line_bid.set_data(x_bid, y_bid)
            self.line_ask.set_data(x_ask, y_ask)
            self.line_pv.set_data(x_pv, y_pv)    
        
            # Setting axes limits -- We want to show change from start (0)
            self.ax1.set_xlim(0, max(line_numbers_safe))
            self.ax2.set_xlim(0, max(line_numbers_safe))
            
            # Update title to contain up-to-date info
            self.fig.suptitle(f'Real-Time Trading Data - Received {data_count} packets')
            self.ax1.set_title(f'Bid/Ask Prices -- Bid: {bid_prices_safe[-1]} | Ask: {ask_prices_safe[-1]}')
            self.ax2.set_title(f'Portfolio Value (in 100,000s) - Current Value: {portfolio_values_safe[-1]}')
            
            return self.line_bid, self.line_ask, self.line_pv
        
        except Exception as e:
            # Graceful error handling - keeps animation running
            print(f"Animation error: {e}")
            return
    
    
    # Main function to create the animated graph
    def run(self):
        ani = animation.FuncAnimation(self.fig, self.animate, interval=100, blit = False, cache_frame_data=False)
        
        try:
            plt.tight_layout()
            plt.show()
        except KeyboardInterrupt:
            print("Graph window closed")
        finally:
            self.running = False
            self.sock.close()


if __name__ == "__main__":
    # Create graph making object and run it 
    grapher = RealTimeGrapher()
    grapher.run()