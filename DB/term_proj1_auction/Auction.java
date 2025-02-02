import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.ResultSet;
import java.sql.PreparedStatement;
import java.sql.DriverManager;
import java.time.LocalDateTime;
import java.sql.Timestamp;
import java.time.format.DateTimeFormatter;
import java.time.Duration;
import java.text. *;
import java.util. *;

public class Auction {
	private static Scanner scanner = new Scanner(System.in);
	private static String username;
	private static Connection conn;

	enum Category {
		ELECTRONICS, 
		BOOKS,
		HOME,
		CLOTHING,
		SPORTINGGOODS,
		OTHERS
	}
	enum Condition {
		NEW,
		LIKE_NEW,
		GOOD,
		ACCEPTABLE 
	}

	private static boolean LoginMenu() {
		String userpass;

		System.out.print("----< User Login >\n" +
				" ** To go back, enter 'back' in user ID.\n" +
				"     user ID: ");
		try{
			username = scanner.next();
			scanner.nextLine();

			if(username.equalsIgnoreCase("back")){
				return false;
			}

			System.out.print("     password: ");
			userpass = scanner.next();
			scanner.nextLine();
		}catch (java.util.InputMismatchException e) {
			System.out.println("Error: Invalid input is entered. Try again.");
			username = null;
			return false;
		}

		/* Your code should come here to check ID and password */ 
		try{
			PreparedStatement pstmt = conn.prepareStatement("select * from auction_user where id = ? and password = ? and isAdmin = 'N'");
			
			pstmt.setString(1, username);
			pstmt.setString(2, userpass);

			ResultSet rset = pstmt.executeQuery();

			if (!rset.next()) {  
				/* If Login Fails */
				System.out.println("Error: Incorrect user name or password");
				username = null;
				pstmt.close();
				rset.close();
				return false;
			}
			
			pstmt.close();
			rset.close();
		}
		catch (SQLException sqle) {         
            System.out.println("SQLException : " + sqle);
			return false;     
        }
        catch (Exception e) {         
            System.out.println("Exception : " + e);
			return false;
        }

		System.out.println("You are successfully logged in.\n");
		return true;
	}

	private static boolean SellMenu() {
		Category category = Category.ELECTRONICS;
		Condition condition = Condition.NEW;
		char choice;
		int price, item_id = 0;
		boolean flag_catg = true, flag_cond = true;
		String description;
		Timestamp timeStamp;

		do{
			System.out.println(
					"----< Sell Item >\n" +
					"---- Choose a category.\n" +
					"    1. Electronics\n" +
					"    2. Books\n" +
					"    3. Home\n" +
					"    4. Clothing\n" +
					"    5. Sporting Goods\n" +
					"    6. Other Categories\n" +
					"    P. Go Back to Previous Menu"
					);

			try {
				choice = scanner.next().charAt(0);;
			}catch (java.util.InputMismatchException e) {
				System.out.println("Error: Invalid input is entered. Try again.");
				continue;
			}

			flag_catg = true;

			switch ((int) choice){
				case '1':
					category = Category.ELECTRONICS;
					continue;
				case '2':
					category = Category.BOOKS;
					continue;
				case '3':
					category = Category.HOME;
					continue;
				case '4':
					category = Category.CLOTHING;
					continue;
				case '5':
					category = Category.SPORTINGGOODS;
					continue;
				case '6':
					category = Category.OTHERS;
					continue;
				case 'p':
				case 'P':
					return false;
				default:
					System.out.println("Error: Invalid input is entered. Try again.");
					flag_catg = false;
					continue;
			}
		}while(!flag_catg);

		do{
			System.out.println(
					"---- Select the condition of the item to sell.\n" +
					"   1. New\n" +
					"   2. Like-new\n" +
					"   3. Used (Good)\n" +
					"   4. Used (Acceptable)\n" +
					"   P. Go Back to Previous Menu"
					);

			try {
				choice = scanner.next().charAt(0);;
				scanner.nextLine();
			}catch (java.util.InputMismatchException e) {
				System.out.println("Error: Invalid input is entered. Try again.");
				continue;
			}

			flag_cond = true;

			switch (choice) {
				case '1':
					condition = Condition.NEW;
					break;
				case '2':
					condition = Condition.LIKE_NEW;
					break;
				case '3':
					condition = Condition.GOOD;
					break;
				case '4':
					condition = Condition.ACCEPTABLE;
					break;
				case 'p':
				case 'P':
					return false;
				default:
					System.out.println("Error: Invalid input is entered. Try again.");
					flag_cond = false;
					continue;
			}
		}while(!flag_cond);

		try {
			System.out.println("---- Description of the item (one line): ");
			description = scanner.nextLine();
			System.out.println("---- Buy-It-Now price: ");

			while (!scanner.hasNextInt()) {
				scanner.next();
				System.out.println("Invalid input is entered. Please enter Buy-It-Now price: ");
			}

			price = scanner.nextInt();
			scanner.nextLine();

			System.out.print("---- Bid closing date and time (YYYY-MM-DD HH:MM): ");
			// you may assume users always enter valid date/time
			String date = scanner.nextLine();  /* "2023-03-04 11:30"; */
			DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");
			LocalDateTime dateTime = LocalDateTime.parse(date, formatter);
			timeStamp = Timestamp.valueOf(dateTime);
		}catch (Exception e) {
			System.out.println("Error: Invalid input is entered. Going back to the previous menu.");
			return false;
		}

		/* TODO: Your code should come here to store the user inputs in your database */
		try{
			PreparedStatement pstmt = conn.prepareStatement("insert into item values(default, ?, ?, ?, ?, ?, ?, 0)");
			
			pstmt.setString(1, category.toString()); // category
			pstmt.setString(2, description); // description
			pstmt.setString(3, condition.toString()); // condition
			pstmt.setString(4, username); // seller_id
			pstmt.setInt(5, price);// buy_it_now_price
			pstmt.setTimestamp(6, timeStamp);// date_post

			pstmt.executeUpdate();

			PreparedStatement pstmt2 = conn.prepareStatement(
				"select id from item " +
				"where category = ? and description = ? and condition = ? and seller_id = ? and " +
				"buy_it_now_price = ? and bid_closing_date = ?"
			);

			pstmt2.setString(1, category.toString());
			pstmt2.setString(2, description);
			pstmt2.setString(3, condition.toString());
			pstmt2.setString(4, username);
			pstmt2.setInt(5, price);
			pstmt2.setTimestamp(6, timeStamp);

			ResultSet rset = pstmt2.executeQuery();

			while(rset.next()){
				item_id = rset.getInt(1);
			}

			PreparedStatement pstmt3 = conn.prepareStatement(
				"insert into bid values(?, 0, ?, null)"
			);
			pstmt3.setInt(1, item_id);
			pstmt3.setString(2, username);
			pstmt3.executeUpdate();

			pstmt3.close();
			rset.close();
			pstmt2.close();
			pstmt.close();
		}
		catch (SQLException sqle) {         
            System.out.println("SQLException : " + sqle);
			return false;     
        }
        catch (Exception e) {         
            System.out.println("Exception : " + e);
			return false;
        }
		

		System.out.println("Your item has been successfully listed.\n");
		return true;
	}

	private static boolean SignupMenu() {
		/* 2. Sign Up */
		String new_username, userpass, isAdmin;
		System.out.print("----< Sign Up >\n" + 
				" ** To go back, enter 'back' in user ID.\n" +
				"---- user name: ");
		try {
			new_username = scanner.next();
			scanner.nextLine();
			if(new_username.equalsIgnoreCase("back")){
				return false;
			}
			System.out.print("---- password: ");
			userpass = scanner.next();
			scanner.nextLine();
			System.out.print("---- In this user an administrator? (Y/N): ");
			isAdmin = scanner.next();
			scanner.nextLine();
		} catch (java.util.InputMismatchException e) {
			System.out.println("Error: Invalid input is entered. Please select again.");
			return false;
		}

		/* TODO: Your code should come here to create a user account in your database */
		try{
			PreparedStatement stmt = conn.prepareStatement(
				"insert into auction_user values (?, ?, ?)"
			);
			stmt.setString(1, new_username);
			stmt.setString(2, userpass);
			stmt.setString(3, isAdmin);
			
			stmt.executeUpdate();
			stmt.close();
		}
		catch(SQLException sqle){
			System.out.println("id or password is duplicated or invalid isAdmin. Please select again.");  
			return false;
		}
		catch (NullPointerException npe) {
			System.out.println("Error: failed to create user account. " + npe.getMessage());
			return false;
		}
		
		System.out.println("Your account has been successfully created.\n");
		return true;
	}

	private static boolean AdminMenu() {
		/* 3. Login as Administrator */
		char choice;
		String adminname, adminpass;
		String keyword, seller;
		System.out.print("----< Login as Administrator >\n" +
				" ** To go back, enter 'back' in user ID.\n" +
				"---- admin ID: ");

		try {
			adminname = scanner.next();
			scanner.nextLine();
			if(adminname.equalsIgnoreCase("back")){
				return false;
			}
			System.out.print("---- password: ");
			adminpass = scanner.nextLine();
			// TODO: check the admin's account and password.
			try{
				PreparedStatement pStmt = conn.prepareStatement("select * from auction_user where id = ? and password = ? and isAdmin = 'Y'");
				
				pStmt.setString(1, adminname);
				pStmt.setString(2, adminpass);

				ResultSet rset = pStmt.executeQuery();

				if (!rset.next()) {  
					/* If Login Fails */
					System.out.println("Error: Incorrect admin name or password or not Admin");
					pStmt.close();
					rset.close();
					return false; 
				}
				
				pStmt.close();
				rset.close();
			}
			catch (SQLException sqle) {         
				System.out.println("SQLException : " + sqle);        
			}
			catch (Exception e) {         
				System.out.println("Exception : " + e);        
			}
		} catch (java.util.InputMismatchException e) {
			System.out.println("Error: Invalid input is entered. Try again.");
			return false;
		}

		boolean login_success = true;

		if(!login_success){
			// login failed. go back to the previous menu.
			return false;
		}

		do {
			System.out.println(
					"----< Admin menu > \n" +
					"    1. Print Sold Items per Category \n" +
					"    2. Print Account Balance for Seller \n" +
					"    3. Print Seller Ranking \n" +
					"    4. Print Buyer Ranking \n" +
					"    P. Go Back to Previous Menu"
					);

			try {
				choice = scanner.next().charAt(0);;
				scanner.nextLine();
			} catch (java.util.InputMismatchException e) {
				System.out.println("Error: Invalid input is entered. Try again.");
				continue;
			}

			if (choice == '1') {
				System.out.println("----Enter Category to search : ");
				keyword = scanner.next();
				scanner.nextLine();
				/*TODO: Print Sold Items per Category */
				System.out.println("sold item       | sold date       | seller ID   | buyer ID   | price    | commissions  ");
				System.out.println("----------------------------------------------------------------------------------");
				
				try{
					PreparedStatement stmt = conn.prepareStatement(
						"select item_id, purchase_date, item.seller_id, buyer_id, amount_paid, amount_paid - amount_due from billing, item where item.category = ?"
					);
					stmt.setString(1, keyword);
					DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");
					ResultSet rset = stmt.executeQuery();
					while(rset.next()){
						System.out.printf("%-15d|%-18s|%-13s|%-13s|%-13.1f|%-13.1f\n",
											rset.getInt(1), rset.getTimestamp(2).toLocalDateTime().format(formatter),
											rset.getString(3), rset.getString(4),
											rset.getFloat(5), rset.getFloat(6));
					}
					stmt.close();
					rset.close();
				}
				catch(SQLException sqle){
					System.out.println("Query Error : " + sqle);
					return false;
				}
				 
				continue;
			} else if (choice == '2') {
				/*TODO: Print Account Balance for Seller */
				System.out.println("---- Enter Seller ID to search : ");
				seller = scanner.next();
				scanner.nextLine();
				System.out.println("sold item       | sold date       | buyer ID   | price    | commissions  ");
				System.out.println("--------------------------------------------------------------------");
				
				try{
					PreparedStatement stmt = conn.prepareStatement(
						"select item.id, purchase_date, buyer_id, amount_paid, amount_paid - amount_due "
						+ "from billing, item "
						+ "where item.seller_id = ? and billing.item_id = item.id"
					);
					stmt.setString(1, seller);

					DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");
					ResultSet rset = stmt.executeQuery();
					while(rset.next()){
						System.out.printf("%-15s|%-18s|%-13s|%-13.1f|%-13.1f\n",
											rset.getString(1), rset.getTimestamp(2).toLocalDateTime().format(formatter),
											rset.getString(3),
											rset.getFloat(4), rset.getFloat(5));
					}
					stmt.close();
					rset.close();
				}
				catch(SQLException sqle){
					System.out.println("Query Error : " + sqle);
					return false;
				}
				
				continue;
			} else if (choice == '3') {
				/*TODO: Print Seller Ranking */
				System.out.println("seller ID   | # of items sold | Total Profit (excluding commissions)");
				System.out.println("--------------------------------------------------------------------");

				try{
					Statement stmt = conn.createStatement();
					ResultSet rset = stmt.executeQuery(
						"select seller_id, count(*), sum(amount_due) " + 
						"from billing " +
						"group by seller_id " +
						"order by sum(amount_due) DESC, count(*) DESC"
					);

					for(int i = 0; i < 10 && rset.next(); i++){
						System.out.printf("%-13s|%-20d|%-16.1f\n",
											rset.getString(1),
											rset.getInt(2),
											rset.getFloat(3) );
					}

					rset.close();
					stmt.close();
				}
				catch(SQLException sqle){
					System.out.println("Query Error : " + sqle);
					return false;
				}

				continue;
			} else if (choice == '4') {
				/*TODO: Print Buyer Ranking */
				System.out.println("buyer ID   | # of items purchased | Total Money Spent ");
				System.out.println("------------------------------------------------------");
				
				try{
					Statement stmt = conn.createStatement();
					ResultSet rset = stmt.executeQuery(
						"select buyer_id, count(*), sum(amount_paid) " + 
						"from billing " +
						"group by buyer_id " +
						"order by sum(amount_paid) DESC, count(*) DESC"
					);

					for(int i = 0; i < 10 && rset.next(); i++){
						System.out.printf("%-13s|%-20d|%-16.1f\n",
											rset.getString(1),
											rset.getInt(2),
											rset.getFloat(3) );
					}

					rset.close();
					stmt.close();
				}
				catch(SQLException sqle){
					System.out.println("Query Error : " + sqle);
					return false;
				}

				continue;
			} else if (choice == 'P' || choice == 'p') {
				return false;
			} else {
				System.out.println("Error: Invalid input is entered. Try again.");
				continue;
			}
		} while(true);
	}

	public static void CheckSellStatus(){
		/* TODO: Check the status of the item the current user is selling */

		System.out.println("item listed in Auction | bidder (buyer ID) | bidding price | bidding date/time \n");
		System.out.println("-------------------------------------------------------------------------------\n");
		try{
					PreparedStatement stmt = conn.prepareStatement(
						"select bid.item_id, bidder, bid_price, date_posted " +
						"from item, bid " +
						"where item.id = bid.item_id and item.seller_id = ? and bid_price <> 0"
					);

					stmt.setString(1, username);
					DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");
					ResultSet rset = stmt.executeQuery();


					while(rset.next()){
						System.out.printf("%-24s|%-18s|%-16.1f|%s\n",
											rset.getInt(1),
											rset.getString(2),
											rset.getFloat(3),
											rset.getTimestamp(4).toLocalDateTime().format(formatter) );
					}

					rset.close();
					stmt.close();
				}
				catch(SQLException sqle){
					System.out.println("Query Error : " + sqle);
				}
	}

	public static boolean BuyItem(){
		Category category = Category.ELECTRONICS;
		Condition condition = Condition.NEW;
		char choice;
		int price;
		String keyword, seller, datePosted, seller_id;
		boolean flag_catg = true, flag_cond = true;
		Timestamp dateposted;

		do {
			System.out.println( "----< Select category > : \n" +
					"    1. Electronics\n"+
					"    2. Books\n" + 
					"    3. Home\n" + 
					"    4. Clothing\n" + 
					"    5. Sporting Goods\n" +
					"    6. Other categories\n" +
					"    7. Any category\n" +
					"    P. Go Back to Previous Menu"
					);

			try {
				choice = scanner.next().charAt(0);;
				scanner.nextLine();
			} catch (java.util.InputMismatchException e) {
				System.out.println("Error: Invalid input is entered. Try again.");
				return false;
			}

			flag_catg = true;

			switch (choice) {
				case '1':
					category = Category.ELECTRONICS;
					break;
				case '2':
					category = Category.BOOKS;
					break;
				case '3':
					category = Category.HOME;
					break;
				case '4':
					category = Category.CLOTHING;
					break;
				case '5':
					category = Category.SPORTINGGOODS;
					break;
				case '6':
					category = Category.OTHERS;
					break;
				case '7':
					break;
				case 'p':
				case 'P':
					return false;
				default:
					System.out.println("Error: Invalid input is entered. Try again.");
					flag_catg = false;
					continue;
			}
		} while(!flag_catg);

		do {

			System.out.println(
					"----< Select the condition > \n" +
					"   1. New\n" +
					"   2. Like-new\n" +
					"   3. Used (Good)\n" +
					"   4. Used (Acceptable)\n" +
					"   P. Go Back to Previous Menu"
					);
			try {
				choice = scanner.next().charAt(0);;
				scanner.nextLine();
			} catch (java.util.InputMismatchException e) {
				System.out.println("Error: Invalid input is entered. Try again.");
				return false;
			}

			flag_cond = true;

			switch (choice) {
				case '1':
					condition = Condition.NEW;
					break;
				case '2':
					condition = Condition.LIKE_NEW;
					break;
				case '3':
					condition = Condition.GOOD;
					break;
				case '4':
					condition = Condition.ACCEPTABLE;
					break;
				case 'p':
				case 'P':
					return false;
				default:
					System.out.println("Error: Invalid input is entered. Try again.");
					flag_cond = false;
					continue;
				}
		} while(!flag_cond);

		try {
			System.out.println("---- Enter keyword to search the description : ");
			keyword = scanner.next();
			scanner.nextLine();

			System.out.println("---- Enter Seller ID to search : ");
			System.out.println(" ** Enter 'any' if you want to see items from any seller. ");
			seller = scanner.next();
			scanner.nextLine();

			System.out.println("---- Enter date posted (YYYY-MM-DD): ");
			System.out.println(" ** This will search items that have been posted after the designated date.");
			datePosted = scanner.next();
			dateposted = Timestamp.valueOf(datePosted + " 00:00:00");
			scanner.nextLine();
		} catch (java.util.InputMismatchException e) {
			System.out.println("Error: Invalid input is entered. Try again.");
			return false;
		}

		/* TODO: Query condition: item category */
		/* TODO: Query condition: item condition */
		/* TODO: Query condition: items whose description match the keyword (use LIKE operator) */
		/* TODO: Query condition: items from a particular seller */
		/* TODO: Query condition: posted date of item */

		String query = "";

		if (seller.equals("any")) {
			query = "select I.id, I.description, I.condition, I.seller_id, I.buy_it_now_price, B.bid_price, B.bidder, extract(day from (I.bid_closing_date - now())), extract(hour from (I.bid_closing_date - now())), I.bid_closing_date " +
					"from item as I natural join bid as B " +
					"where I.id = B.item_id and I.condition = ? and I.category = ? and I.description like ? and I.bid_closing_date > now() and I.bid_closing_date > ? and I.status = 0";
		}
		else{
			query = "select I.id, I.description, I.condition, I.seller_id, I.buy_it_now_price, B.bid_price, B.bidder, extract(day from (I.bid_closing_date - now())), extract(hour from (I.bid_closing_date - now())), I.bid_closing_date " +
					"from item as I natural join bid as B " +
					"where I.id = B.item_id and I.condition = ? and I.category = ? and I.description like ? and I.bid_closing_date > now() and I.bid_closing_date > ? and I.seller_id = ? and I.status = 0";
		}

		/* TODO: List all items that match the query condition */
		System.out.println("Item ID | Item description | Condition | Seller | Buy-It-Now | Current Bid | highest bidder | Time left | bid close");
		System.out.println("-------------------------------------------------------------------------------------------------------");
		
		try{
			PreparedStatement pstmt = conn.prepareStatement(query);
			pstmt.setString(1, condition.toString());
			pstmt.setString(2, category.toString());
			pstmt.setString(3, "%" + keyword + "%");
			pstmt.setTimestamp(4, dateposted);
			if (!seller.equals("any")) {
				pstmt.setString(5, seller);
			}

			DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");
			ResultSet rset = pstmt.executeQuery();

			if(!rset.next()){
				System.out.println("is empty");
				rset.close();
				pstmt.close();
				return false;
			}

			do {
				String time_left = rset.getInt(8) + " day " + rset.getInt(9) + " hrs";
				
				System.out.printf("%-8s| %-17s| %-10s| %-8s| %-10d| %-10d| %-14s| %-12s| %s\n",
											rset.getString(1),
											rset.getString(2),
											rset.getString(3),
											rset.getString(4),
											rset.getInt(5),
											rset.getInt(6),
											rset.getString(7),
											time_left,
											rset.getTimestamp(10).toLocalDateTime().format(formatter)
				);
			} while(rset.next());

			rset.close();
			pstmt.close();
		}
		catch(SQLException sqle){
			System.out.println("Query Error : " + sqle);
			return false;
		}

		System.out.println("---- Select Item ID to buy or bid: ");
		int choice_id;
		try {
			choice_id= scanner.nextInt();
			scanner.nextLine();
			System.out.println("     Price: ");
			price = scanner.nextInt();
			scanner.nextLine();
		} catch (java.util.InputMismatchException e) {
			System.out.println("Error: Invalid input is entered. Try again.");
			return false;
		}

		/* TODO: Buy-it-now or bid: If the entered price is higher or equal to Buy-It-Now price, the bid ends. */
		/* Even if the bid price is higher than the Buy-It-Now price, the buyer pays the B-I-N price. */

		try{
			PreparedStatement pstmt = conn.prepareStatement(
				"select distinct buy_it_now_price, bid_price, item.seller_id " +
				"from item, bid " + 
				"where item.id = ? and item.id = bid.item_id"
			);
			pstmt.setInt(1, choice_id);
			ResultSet rset = pstmt.executeQuery();

			while(rset.next()){
				if(rset.getInt(2) < price){
					if(rset.getInt(1) <= price){
						price = rset.getInt(1);
						seller_id = rset.getString(3);

						PreparedStatement insert_stmt = conn.prepareStatement(
							"insert into billing " +
							"values (?, ?, ?, CURRENT_TIMESTAMP, ?, ?)"
						);
						insert_stmt.setInt(1, choice_id);
						insert_stmt.setString(2, seller_id);
						insert_stmt.setString(3, username);
						insert_stmt.setInt(4, (int)Math.floor(price * 0.9));
						insert_stmt.setInt(5, price);
						insert_stmt.executeUpdate();


						// insert_stmt = conn.prepareStatement(
						// 	"delete from bid " + 
						// 	"where item_id = ? "
						// );
						
						// insert_stmt.setInt(1, choice_id);
						// insert_stmt.executeUpdate();

						insert_stmt = conn.prepareStatement(
							"update item " +
							"set status = 1" +
							"where id = ?"
						);
						insert_stmt.setInt(1, choice_id);
						insert_stmt.executeUpdate();

						insert_stmt = conn.prepareStatement(
							"insert into bid " +
							"values (?, ?, ?, CURRENT_TIMESTAMP)"
						);
						insert_stmt.setInt(1, choice_id);
						insert_stmt.setInt(2, price);
						insert_stmt.setString(3, username);
						insert_stmt.executeUpdate();

						System.out.println("Congratulations, the item is yours now.\n"); 
						insert_stmt.close();
					}
					else{
						PreparedStatement insert_stmt = conn.prepareStatement(
							"insert into bid " +
							"values (?, ?, ?, CURRENT_TIMESTAMP)"
						);
						insert_stmt.setInt(1, choice_id);
						insert_stmt.setInt(2, price);
						insert_stmt.setString(3, username);
						insert_stmt.executeUpdate();
						
						insert_stmt.close();
						System.out.println("Congratulations, you are the highest bidder.\n"); 
					}
				}
				else{
					System.out.println("lower than current bid");
				}
			}
			
			pstmt.close();
			rset.close();
		}
		catch(SQLException sqle){
			System.out.println("Query Error : " + sqle);
			return false;
		}
		return true;
	}

	public static void CheckBuyStatus(){
		/* TODO: Check the status of the item the current buyer is bidding on */
		/* Even if you are outbidded or the bid closing date has passed, all the items this user has bidded on must be displayed */

		System.out.println("item ID   | item description   | highest bidder | highest bidding price | your bidding price | bid closing date/time");
		System.out.println("--------------------------------------------------------------------------------------------------------------------");
		try{
			PreparedStatement stmt = conn.prepareStatement(
				"select max_data.item_id, description, max_bid_price, bidder, bid_closing_date, my_bid_price " +
				"from ( select item_id, max(bid_price) as max_bid_price " +
				"from bid group by item_id ) max_data " + 
				"inner join bid on max_data.item_id = bid.item_id and max_data.max_bid_price = bid.bid_price  " +
				"left outer join item on max_data.item_id = item.id " +
				"inner join ( select item_id, bid_price as my_bid_price " +
				"from bid where bidder = ? and bid_price <> 0" +
				") my_bid on max_data.item_id = my_bid.item_id "
			);

			stmt.setString(1, username);
			ResultSet rset = stmt.executeQuery();
			DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");

			while(rset.next()){
				System.out.printf(" %-9d| %-19s| %-15s| %-22d| %-15d| %s\n",
											rset.getInt(1),
											rset.getString(2),
											rset.getString(4),
											rset.getInt(3),
											rset.getInt(6),
											rset.getTimestamp(5).toLocalDateTime().format(formatter)
				);
			}

			stmt.close();
			rset.close();
		}
		catch(SQLException sqle){
			System.out.println("Query Error : " + sqle);
		}
	}

	public static void CheckAccount(){
		/* TODO: Check the balance of the current user.  */
		System.out.println("[Sold Items] \n");
		System.out.println("item category  | item ID   | sold date | sold price  | buyer ID | commission  ");
		System.out.println("------------------------------------------------------------------------------");
		try{
			PreparedStatement stmt = conn.prepareStatement(
				"select distinct category, item.id, purchase_date, amount_due, buyer_id, amount_paid - amount_due " +
				"from billing, item " +
				"where billing.item_id = item.id and item.seller_id = ?"
			);

			stmt.setString(1, username);
			ResultSet rset = stmt.executeQuery();

			DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");

			while(rset.next()){
				System.out.printf(" %-15s| %-10d| %-10s| %-12d| %-9s| %d\n",
											rset.getString(1),
											rset.getInt(2),
											rset.getTimestamp(3).toLocalDateTime().format(formatter),
											rset.getInt(4),
											rset.getString(5),
											rset.getInt(6)
				);
			}

			stmt.close();
			rset.close();
		}
		catch(SQLException sqle){
			System.out.println("Query Error : " + sqle);
		}

		System.out.println("[Purchased Items] \n");
		System.out.println("item category  | item ID   | purchased date | puchased price  | seller ID ");
		System.out.println("--------------------------------------------------------------------------");
		try{
			PreparedStatement stmt = conn.prepareStatement(
				"select distinct category, item.id, purchase_date, amount_paid, item.seller_id " +
				"from billing, item " +
				"where billing.item_id = item.id and billing.buyer_id = ?"
			);

			stmt.setString(1, username);
			ResultSet rset = stmt.executeQuery();

			DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");

			while(rset.next()){
				System.out.printf(" %-15s| %-10d| %-10s| %-12d| %-9s\n",
											rset.getString(1),
											rset.getInt(2),
											rset.getTimestamp(3).toLocalDateTime().format(formatter),
											rset.getInt(4),
											rset.getString(5)
				);
			}

			stmt.close();
			rset.close();
		}
		catch(SQLException sqle){
			System.out.println("Query Error : " + sqle);
		}
	}

	public static void main(String[] args) {
		char choice;
		boolean ret;

		if(args.length<2){
			System.out.println("Usage: java Auction postgres_id password");
			System.exit(1);
		}


		try{
            conn = DriverManager.getConnection("jdbc:postgresql://localhost/"+args[0], args[0], args[1]); 
            // Connection conn = DriverManager.getConnection("jdbc:postgresql://localhost/bnam", "bnam", "changethis"); 
		}
		catch(SQLException e){
			System.out.println("SQLException : " + e);	
			System.exit(1);
		}
		catch (NullPointerException npe) {
			System.out.println("Error: failed to get psql host " + npe.getMessage());
			System.exit(1);
		}

		do {
			username = null;
			System.out.println(
					"----< Login menu >\n" + 
					"----(1) Login\n" +
					"----(2) Sign up\n" +
					"----(3) Login as Administrator\n" +
					"----(Q) Quit"
					);

			try {
				choice = scanner.next().charAt(0);;
				scanner.nextLine();
			} catch (java.util.InputMismatchException e) {
				System.out.println("Error: Invalid input is entered. Try again.");
				continue;
			}

			try {
				switch ((int) choice) {
					case '1':
						ret = LoginMenu();
						if(!ret) continue;
						break;
					case '2':
						ret = SignupMenu();
						if(!ret) continue;
						break;
					case '3':
						ret = AdminMenu();
						if(!ret) continue;
					case 'q':
					case 'Q':
						System.out.println("Good Bye");
						/* TODO: close the connection and clean up everything here */
						conn.close();
						System.exit(1);
					default:
						System.out.println("Error: Invalid input is entered. Try again.");
				}
			} catch (SQLException e) {
				System.out.println("SQLException : " + e);	
			}
		} while (username==null || username.equalsIgnoreCase("back"));  

		// logged in as a normal user 
		do {
			System.out.println(
					"---< Main menu > :\n" +
					"----(1) Sell Item\n" +
					"----(2) Status of Your Item Listed on Auction\n" +
					"----(3) Buy Item\n" +
					"----(4) Check Status of your Bid \n" +
					"----(5) Check your Account \n" +
					"----(Q) Quit"
					);

			try {
				choice = scanner.next().charAt(0);;
				scanner.nextLine();
			} catch (java.util.InputMismatchException e) {
				System.out.println("Error: Invalid input is entered. Try again.");
				continue;
			}

			try{
				switch (choice) {
					case '1':
						ret = SellMenu();
						if(!ret) continue;
						break;
					case '2':
						CheckSellStatus();
						break;
					case '3':
						ret = BuyItem();
						if(!ret) continue;
						break;
					case '4':
						CheckBuyStatus();
						break;
					case '5':
						CheckAccount();
						break;
					case 'q':
					case 'Q':
						System.out.println("Good Bye");
						/* TODO: close the connection and clean up everything here */
						conn.close();
						System.exit(1);
				}
			} catch (SQLException e) {
				System.out.println("SQLException : " + e);	
				System.exit(1);
			}
		} while(true);
	} // End of main 
} // End of class