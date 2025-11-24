#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ORDERS 10
#define HISTORY_FILE "Order_History.txt"

//Struct for Order
typedef struct {
    char name[20];
    int quantity;
    int price; 
    int subtotal;
} OrderedItem;

const char *MENU_ITEMS[] = {"Samosa", "Cold-drink", "Pastry", "Tea"};
const int MENU_PRICES[] = {30, 90, 40, 20};
const int MENU_SIZE = 4;

//Function Prototypes
void print_menu();
int print_formatted_receipt(FILE *stream, OrderedItem orders[], int order_count);
void write_history_to_file(OrderedItem orders[], int order_count, int grand_total);

int main() {

    OrderedItem current_orders[MAX_ORDERS];
    int order_count = 0;
    int choice = 0;
    int quantity = 0;
    int grand_total = 0;

    printf("\n\n--- Welcome to Nedian Canteen Counter ---\n");

    // Main ordering loop
    while (1) {
        // 1. Display the menu
        print_menu();

        printf("Enter your choice (1-%d): ", MENU_SIZE + 1);
        if (scanf("%d", &choice) != 1) {
            // Handle invalid input (e.g., non-numeric)
            printf("Invalid input. Please enter a number.\n");
            // Clear input buffer
            while (getchar() != '\n');
            continue;
        }

        //Ending
        if (choice == MENU_SIZE + 1) {
            if (order_count > 0) {
                printf("\n--- Order Finalized ---\n");
                break;
            } else {
                printf("No items added. Please select an item.\n");
                continue;
            }
        }

        //Processing item selection
        if (choice >= 1 && choice <= MENU_SIZE) {
            if (order_count >= MAX_ORDERS) {
                printf("Maximum number of items reached for this order.\n");
                continue;
            }

            printf("You selected %s (Rs. %d).\n", MENU_ITEMS[choice - 1], MENU_PRICES[choice - 1]);
            printf("Enter quantity: ");

            if (scanf("%d", &quantity) != 1 || quantity <= 0) {
                printf("Invalid quantity. Please enter a positive number.\n");
                // Clear input buffer
                while (getchar() != '\n');
                continue;
            }

            // Store order details
            OrderedItem *new_item = &current_orders[order_count];
            strcpy(new_item->name, MENU_ITEMS[choice - 1]);
            new_item->quantity = quantity;
            new_item->price = MENU_PRICES[choice - 1];
            new_item->subtotal = quantity * new_item->price;

            order_count++;
            printf("Added %dx %s to your order.\n\n", quantity, new_item->name);

        } else {
            printf("Invalid choice. Please select a valid menu number.\n");
        }
    }

    // --- Receipt Generation ---
    if (order_count > 0) {
        // 1. Print receipt to console
        printf("\n\n******************** RECEIPT ********************\n");
        grand_total = print_formatted_receipt(stdout, current_orders, order_count);
        printf("*************************************************\n");

        // 2. Write order history to file
        write_history_to_file(current_orders, order_count, grand_total);
        printf("\nOrder history successfully appended to '%s'.\n", HISTORY_FILE);
    } else {
        printf("\nOrder cancelled. Goodbye!\n");
    }

    return 0;
}

// Displays the current menu options
void print_menu() {
    printf("\n--- Menu ---\n");
    for (int i = 0; i < MENU_SIZE; i++) {
        printf("%d. %-10s (Rs. %d)\n", i + 1, MENU_ITEMS[i], MENU_PRICES[i]);
    }
    printf("%d. Done & Print Receipt\n", MENU_SIZE + 1);
    printf("------------\n");
}


// Prints the formatted receipt to any given stream (like stdout or a file)
// Returns the calculated grand total
int print_formatted_receipt(FILE *stream, OrderedItem orders[], int order_count) {
    int grand_total = 0;

    // Print Header
    fprintf(stream, "\nOrder:\n");
    fprintf(stream, "%-12s %-5s %s\n", "Item", "Qty", "Price"); // Headers
    fprintf(stream, "--------------------\n");

    // Print Line Items and calculate Total
    for (int i = 0; i < order_count; i++) {
        grand_total += orders[i].subtotal;
        // Format: ItemName    Qtyx     Subtotal
        // Note: orders[i].subtotal is the total price for that item (e.g., 2 * 30 = 60)
        fprintf(stream, "%-12s %dx     %d\n", 
                orders[i].name, 
                orders[i].quantity, 
                orders[i].subtotal);
    }

    // Print Footer (Total)
    fprintf(stream, "--------------------\n");
    fprintf(stream, "Total: Rs. %d/-\n", grand_total);
    
    return grand_total;
}

// Handles opening the file and writing the history entry
void write_history_to_file(OrderedItem orders[], int order_count, int grand_total) {
    // Open the file in append mode ("a")
    FILE *fp = fopen(HISTORY_FILE, "a");

    if (fp == NULL) {
        // If file opening fails, print an error and exit the function
        perror("Error opening history file");
        return;
    }

    // Write the main history header
    fprintf(fp, "\n\nNedian Canteen Order History:\n");
    fprintf(fp, "=============================\n");

    // Use the helper function to write the formatted order details
    print_formatted_receipt(fp, orders, order_count);

    // Write the separator for the next order
    fprintf(fp, "=====================================================\n");

    // Close the file to ensure data is saved
    fclose(fp);
}