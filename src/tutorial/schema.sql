create table Users (
	username    EmailAddress,
	realname    text,
	primary key (username)
);

create table Sessions (
	sessionID   integer,
	username    EmailAddress references Users(username),
	loggedin    timestamp,
	primary key (sessionID)
);


