#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_ORDERS 20
#define HISTORY_FILE "Order_History.txt"
#define CLEAR_SCREEN() printf("\033[2J\033[H")
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_RED "\033[1;31m"

// Struct for Order
typedef struct {
    char name[20];
    int quantity;
    int price; 
    int subtotal;
} OrderedItem;

const char *MENU_ITEMS[] = {"Samosa", "Cold-drink", "Pastry", "Tea", "Burger", "Pizza Slice"};
const int MENU_PRICES[] = {30, 90, 40, 20, 150, 120};
const int MENU_SIZE = 6;

// Function Prototypes
void print_banner();
void print_menu();
void view_current_order(OrderedItem orders[], int order_count);
void modify_order(OrderedItem orders[], int *order_count);
void apply_discount(int *grand_total);
int print_formatted_receipt(FILE *stream, OrderedItem orders[], int order_count, int discount);
void write_history_to_file(OrderedItem orders[], int order_count, int grand_total, int discount);
void view_order_history();
void get_customer_info(char *name);
void print_loading(const char *message);
int confirm_action(const char *message);
void pause_screen();

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif
    OrderedItem current_orders[MAX_ORDERS];
    int order_count = 0;
    int choice = 0;
    int quantity = 0;
    int grand_total = 0;
    int discount_percent = 0;
    char customer_name[50] = "Guest";

    CLEAR_SCREEN();
    print_banner();
    
    // Get customer information
    printf("%s\nWould you like to provide your details? (1-Yes, 0-No): %s", COLOR_CYAN, COLOR_RESET);
    int provide_info;
    if (scanf("%d", &provide_info) == 1 && provide_info == 1) {
        get_customer_info(customer_name);
    }
    while (getchar() != '\n'); // Clear buffer

    // Main ordering loop
    while (1) {
        print_menu();
        
        printf("\n%s[Current Order Items: %d/%d]%s\n", COLOR_YELLOW, order_count, MAX_ORDERS, COLOR_RESET);
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("%sInvalid input. Please enter a number.%s\n", COLOR_RED, COLOR_RESET);
            while (getchar() != '\n');
            pause_screen();
            continue;
        }

        // Menu options
        if (choice == MENU_SIZE + 1) { // View Current Order
            view_current_order(current_orders, order_count);
            pause_screen();
            continue;
        }
        else if (choice == MENU_SIZE + 2) { // Modify Order
            if (order_count > 0) {
                modify_order(current_orders, &order_count);
            } else {
                printf("%sNo items in order to modify!%s\n", COLOR_RED, COLOR_RESET);
                pause_screen();
            }
            continue;
        }
        else if (choice == MENU_SIZE + 3) { // View History
            view_order_history();
            pause_screen();
            continue;
        }
        else if (choice == MENU_SIZE + 4) { // Finalize Order
            if (order_count > 0) {
                if (confirm_action("Are you sure you want to finalize the order?")) {
                    printf("\n%s--- Order Finalized ---%s\n", COLOR_GREEN, COLOR_RESET);
                    break;
                }
            } else {
                printf("%sNo items added. Please select an item.%s\n", COLOR_RED, COLOR_RESET);
                pause_screen();
            }
            continue;
        }
        else if (choice == MENU_SIZE + 5) { // Exit
            if (confirm_action("Are you sure you want to exit without ordering?")) {
                printf("%s\nThank you for visiting Nedian Canteen! Goodbye!%s\n", COLOR_CYAN, COLOR_RESET);
                return 0;
            }
            continue;
        }

        // Processing item selection
        if (choice >= 1 && choice <= MENU_SIZE) {
            if (order_count >= MAX_ORDERS) {
                printf("%sMaximum number of items reached!%s\n", COLOR_RED, COLOR_RESET);
                pause_screen();
                continue;
            }

            printf("\n%sYou selected: %s (Rs. %d)%s\n", COLOR_GREEN, MENU_ITEMS[choice - 1], MENU_PRICES[choice - 1], COLOR_RESET);
            printf("Enter quantity: ");

            if (scanf("%d", &quantity) != 1 || quantity <= 0) {
                printf("%sInvalid quantity. Please enter a positive number.%s\n", COLOR_RED, COLOR_RESET);
                while (getchar() != '\n');
                pause_screen();
                continue;
            }

            // Store order details
            OrderedItem *new_item = &current_orders[order_count];
            strcpy(new_item->name, MENU_ITEMS[choice - 1]);
            new_item->quantity = quantity;
            new_item->price = MENU_PRICES[choice - 1];
            new_item->subtotal = quantity * new_item->price;

            order_count++;
            print_loading("Adding item to cart");
            printf("%s✓ Added %dx %s to your order (Rs. %d)%s\n\n", 
                   COLOR_GREEN, quantity, new_item->name, new_item->subtotal, COLOR_RESET);
            pause_screen();

        } else {
            printf("%sInvalid choice. Please select a valid option.%s\n", COLOR_RED, COLOR_RESET);
            pause_screen();
        }
    }

    // --- Receipt Generation ---
    if (order_count > 0) {
        // Calculate initial total
        for (int i = 0; i < order_count; i++) {
            grand_total += current_orders[i].subtotal;
        }

        // Apply discount
        apply_discount(&discount_percent);
        int discount_amount = (grand_total * discount_percent) / 100;
        int final_total = grand_total - discount_amount;

        // Print receipt to console
        CLEAR_SCREEN();
        printf("\n%s", COLOR_CYAN);
        printf("╔════════════════════════════════════════════════╗\n");
        printf("║           NEDIAN CANTEEN - RECEIPT             ║\n");
        printf("╚════════════════════════════════════════════════╝\n");
        printf("%s", COLOR_RESET);
        
        printf("\nCustomer: %s\n", customer_name);
        
        time_t now = time(NULL);
        printf("Date: %s", ctime(&now));
        
        print_formatted_receipt(stdout, current_orders, order_count, discount_percent);
        
        printf("\n%s", COLOR_CYAN);
        printf("\n%sThank you for your order, %s! Please visit again!%s\n", COLOR_GREEN, customer_name, COLOR_RESET);
        printf("════════════════════════════════════════════════\n");
        printf("%s", COLOR_RESET);

        // Write order history to file
        write_history_to_file(current_orders, order_count, final_total, discount_percent);
        printf("\n%sOrder history saved to '%s'%s\n", COLOR_GREEN, HISTORY_FILE, COLOR_RESET);
    }

    return 0;
}

void print_banner() {
    printf("%s", COLOR_CYAN);
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║                                                ║\n");
    printf("║        🍔 NEDIAN CANTEEN COUNTER 🍕            ║\n");
    printf("║                                                ║\n");
    printf("║         Your Friendly Campus Cafe              ║\n");
    printf("║                                                ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    printf("%s\n", COLOR_RESET);
}

void print_menu() {
    CLEAR_SCREEN();
    printf("%s", COLOR_YELLOW);
    printf("\n╔═══════════ MENU ════════════╗\n");
    printf("%s", COLOR_RESET);
    
    for (int i = 0; i < MENU_SIZE; i++) {
        printf("   %s%d.%s %-15s Rs. %-4d\n", COLOR_GREEN, i + 1, COLOR_RESET, MENU_ITEMS[i], MENU_PRICES[i]);
    }
    
    printf("%s", COLOR_YELLOW);
    printf("╠════════════════════════════╣\n");
    printf(" %s", COLOR_RESET);
    printf("  %s%d.%s View Current Order\n", COLOR_CYAN, MENU_SIZE + 1, COLOR_RESET);
    printf("   %s%d.%s Modify Order\n", COLOR_CYAN, MENU_SIZE + 2, COLOR_RESET);
    printf("   %s%d.%s View Order History\n", COLOR_CYAN, MENU_SIZE + 3, COLOR_RESET);
    printf("   %s%d.%s Done & Print Receipt\n", COLOR_GREEN, MENU_SIZE + 4, COLOR_RESET);
    printf("   %s%d.%s Exit\n", COLOR_RED, MENU_SIZE + 5, COLOR_RESET);
    printf("%s", COLOR_YELLOW);
    printf("╚════════════════════════════╝\n");
    printf("%s", COLOR_RESET);
}

void view_current_order(OrderedItem orders[], int order_count) {
    CLEAR_SCREEN();
    printf("\n%s=== CURRENT ORDER ===%s\n\n", COLOR_CYAN, COLOR_RESET);
    
    if (order_count == 0) {
        printf("%sNo items in cart yet.%s\n", COLOR_YELLOW, COLOR_RESET);
        return;
    }
    
    printf("%-4s %-15s %-8s %-8s %s\n", "No.", "Item", "Qty", "Price", "Subtotal");
    printf("──────────────────────────────────────────────\n");
    
    int total = 0;
    for (int i = 0; i < order_count; i++) {
        printf("%-4d %-15s %-8d %-8d Rs. %d\n", 
               i + 1, orders[i].name, orders[i].quantity, 
               orders[i].price, orders[i].subtotal);
        total += orders[i].subtotal;
    }
    
    printf("──────────────────────────────────────────────\n");
    printf("%sTotal: Rs. %d%s\n", COLOR_GREEN, total, COLOR_RESET);
}

void modify_order(OrderedItem orders[], int *order_count) {
    view_current_order(orders, *order_count);
    
    printf("\nEnter item number to remove (0 to cancel): ");
    int item_num;
    
    if (scanf("%d", &item_num) != 1 || item_num < 0 || item_num > *order_count) {
        printf("%sInvalid selection.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }
    
    if (item_num == 0) return;
    
    printf("%sRemoving %s from order...%s\n", COLOR_YELLOW, orders[item_num - 1].name, COLOR_RESET);
    
    // Shift items down
    for (int i = item_num - 1; i < *order_count - 1; i++) {
        orders[i] = orders[i + 1];
    }
    (*order_count)--;
    
    printf("%s✓ Item removed successfully!%s\n", COLOR_GREEN, COLOR_RESET);
}

void apply_discount(int *discount_percent) {
    printf("\n%sApply discount? (1-Yes, 0-No): %s", COLOR_CYAN, COLOR_RESET);
    int apply;
    if (scanf("%d", &apply) == 1 && apply == 1) {
        printf("Enter discount percentage (0-50): ");
        if (scanf("%d", discount_percent) == 1 && *discount_percent >= 0 && *discount_percent <= 50) {
            printf("%s✓ %d%% discount applied!%s\n", COLOR_GREEN, *discount_percent, COLOR_RESET);
        } else {
            *discount_percent = 0;
            printf("%sInvalid discount. No discount applied.%s\n", COLOR_RED, COLOR_RESET);
        }
    }
}

int print_formatted_receipt(FILE *stream, OrderedItem orders[], int order_count, int discount) {
    int grand_total = 0;

    fprintf(stream, "\n%-15s %-8s %s\n", "Item", "Qty", "Amount");
    fprintf(stream, "────────────────────────────────────\n");

    for (int i = 0; i < order_count; i++) {
        grand_total += orders[i].subtotal;
        fprintf(stream, "%-15s %dx       Rs. %d\n", 
                orders[i].name, orders[i].quantity, orders[i].subtotal);
    }

    fprintf(stream, "────────────────────────────────────\n");
    fprintf(stream, "Subtotal:                Rs. %d\n", grand_total);
    
    if (discount > 0) {
        int discount_amount = (grand_total * discount) / 100;
        fprintf(stream, "Discount (%d%%):          -Rs. %d\n", discount, discount_amount);
        grand_total -= discount_amount;
    }
    
    fprintf(stream, "────────────────────────────────────\n");
    fprintf(stream, "TOTAL:                   Rs. %d/-\n", grand_total);
    
    return grand_total;
}

void write_history_to_file(OrderedItem orders[], int order_count, int grand_total, int discount) {
    FILE *fp = fopen(HISTORY_FILE, "a");
    if (fp == NULL) {
        perror("Error opening history file");
        return;
    }

    time_t now = time(NULL);
    fprintf(fp, "\n\n═══════════════════════════════════════════════\n");
    fprintf(fp, "NEDIAN CANTEEN - ORDER HISTORY\n");
    fprintf(fp, "Date: %s", ctime(&now));
    fprintf(fp, "═══════════════════════════════════════════════\n");

    print_formatted_receipt(fp, orders, order_count, discount);

    fprintf(fp, "═══════════════════════════════════════════════\n");
    fclose(fp);
}

void view_order_history() {
    CLEAR_SCREEN();
    printf("\n%s=== ORDER HISTORY ===%s\n\n", COLOR_CYAN, COLOR_RESET);
    
    FILE *fp = fopen(HISTORY_FILE, "r");
    if (fp == NULL) {
        printf("%sNo order history found.%s\n", COLOR_YELLOW, COLOR_RESET);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    fclose(fp);
}

void get_customer_info(char *name) {
    printf("\nEnter your name: ");
    while (getchar() != '\n'); // Clear buffer
    fgets(name, 50, stdin);
    name[strcspn(name, "\n")] = 0; // Remove newline
    
}

void print_loading(const char *message) {
    printf("%s%s", COLOR_YELLOW, message);
    for (int i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        for (int j = 0; j < 100000000; j++); // Simple delay
    }
    printf("%s\n", COLOR_RESET);
}

int confirm_action(const char *message) {
    printf("\n%s%s (1-Yes, 0-No): %s", COLOR_YELLOW, message, COLOR_RESET);
    int confirm;
    if (scanf("%d", &confirm) == 1 && confirm == 1) {
        return 1;
    }
    return 0;
}

void pause_screen() {
    printf("\n%sPress Enter to continue...%s", COLOR_CYAN, COLOR_RESET);
    while (getchar() != '\n');
    getchar();
}