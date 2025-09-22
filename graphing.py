import socket
import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import threading
import time
import re

class RealTimeGrapher:
    def __init__(self):
        # Data storage
        self.bid_prices = deque(maxlen=1000000)
        self.ask_prices = deque(maxlen=1000000)
        self.portfolio_values = deque(maxlen=1000000)
        self.line_numbers = deque(maxlen=1000000)
        
        # Socket setup
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            self.sock.bind(('127.0.0.1', 8888))
            print("✓ Successfully bound to port 8888")
        except Exception as e:
            print(f"✗ Failed to bind to port 8888: {e}")
            return
            
        self.sock.settimeout(0.1)
        
        # Plot setup
        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(12, 8))
        self.fig.suptitle('Real-Time Trading Data - Waiting for data...')
        
        # Start data receiver thread
        self.running = True
        self.data_received_count = 0
        self.data_thread = threading.Thread(target=self.receive_data, daemon=True)
        self.data_thread.start()
        
        print("Python grapher started - waiting for C program data...")

    def parse_json_messages(self, data_str):
        """Parse potentially multiple JSON messages from a single UDP packet"""
        messages = []
        
        # Try to find all JSON objects in the string
        # Look for patterns like {"..."}
        json_pattern = r'\{[^}]*\}'
        matches = re.findall(json_pattern, data_str)
        
        for match in matches:
            try:
                data_obj = json.loads(match)
                messages.append(data_obj)
                print(f"✓ Parsed JSON: Line {data_obj.get('line', '?')}, "
                      f"Bid: {data_obj.get('bid', 0):.6f}")
            except json.JSONDecodeError as e:
                print(f"✗ Failed to parse JSON fragment: '{match}' - {e}")
        
        return messages

    def receive_data(self):
        while self.running:
            try:
                data, addr = self.sock.recvfrom(1024)
                data_str = data.decode('utf-8').strip()
                
                # Parse CSV format: line,bid,ask,portfolio
                parts = data_str.split(',')
                if len(parts) == 4:
                    line_num = int(parts[0])
                    bid = float(parts[1])
                    ask = float(parts[2])
                    portfolio = float(parts[3])
                    
                    self.line_numbers.append(line_num)
                    self.bid_prices.append(bid)
                    self.ask_prices.append(ask)
                    self.portfolio_values.append(portfolio)
                    self.data_received_count += 1
                    
                    #print(f"✓ Received: Line {line_num}, Bid {bid:.6f}, Ask {ask:.6f}")
                    time.sleep(0.000001)
            except:
                pass

    def animate(self, frame):
        self.fig.suptitle(f'Real-Time Trading Data - Received {self.data_received_count} packets')
        
        if len(self.bid_prices) == 0:
            self.ax1.clear()
            self.ax1.text(0.5, 0.5, 'Waiting for data...', 
                        ha='center', va='center', transform=self.ax1.transAxes)
            self.ax2.clear()
            self.ax2.text(0.5, 0.5, 'Make sure C program is running', 
                        ha='center', va='center', transform=self.ax2.transAxes)
            return
        
        # Clear and redraw plots
        self.ax1.clear()
        self.ax2.clear()
        
        if len(self.bid_prices) > 1:
            self.ax1.plot(list(self.line_numbers), list(self.bid_prices), 
                        'g-', label='Bid Price', linewidth=1)
            self.ax1.plot(list(self.line_numbers), list(self.ask_prices), 
                        'r-', label='Ask Price', linewidth=1)
            
            # Adding stable line for where support/resistance lines are
            self.ax1.axhline(y=1.35400, color='black', linestyle='--', linewidth=2, alpha=0.7, label='Resistance Value')
            self.ax1.axhline(y=1.34500, color='black', linestyle='--', linewidth=2, alpha=0.7, label='Support Value')
            
            self.ax1.set_title(f'Bid/Ask Prices ({len(self.bid_prices)} points)')
            self.ax1.set_ylabel('Price')
            
            # SET FIXED Y-AXIS LIMITS FOR PRICE
            self.ax1.set_ylim(1.33, 1.36)  # Adjust these values for your data range
            
            self.ax1.legend()
            self.ax1.grid(True, alpha=0.3)
        
        if len(self.portfolio_values) > 1:
            self.ax2.plot()
            self.ax2.plot(list(self.line_numbers), list(self.portfolio_values), 
                        'b-', label='Portfolio Value', linewidth=2)
            
            # Adding stable line for where account balance began
            self.ax2.axhline(y=10, color='orange', linestyle='--', linewidth=2, alpha=0.7, label='Starting Balance')
            
            self.ax2.set_title(f'Portfolio Value (in 100,000s) ({len(self.portfolio_values)} points)')
            self.ax2.set_xlabel('Line Number')
            self.ax2.set_ylabel('Portfolio Value ($)')
            
            # SET FIXED Y-AXIS LIMITS FOR PORTFOLIO
            self.ax2.set_ylim(9.5, 10.5)  # Adjust these values for your portfolio range
            
            self.ax2.legend()
            self.ax2.grid(True, alpha=0.3)

    def run(self):
        ani = animation.FuncAnimation(self.fig, self.animate, interval=200, cache_frame_data=False)
        
        try:
            plt.tight_layout()
            plt.show()
        except KeyboardInterrupt:
            print("Graph window closed")
        finally:
            self.running = False
            self.sock.close()

if __name__ == "__main__":
    grapher = RealTimeGrapher()
    grapher.run()